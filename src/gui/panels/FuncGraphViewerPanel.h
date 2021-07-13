#pragma once


namespace GUI
{
	static void RenderAddress(uint64_t offset) {
		using namespace Helper::String;
		const auto offsetStr = NumberToHex(offset + (static_cast<uint64_t>(1) << 63)).substr(1);
		Text::Text("base + " + offsetStr).show();
	}

	class Canvas
		: public Control,
		public Attribute::Id
	{
	protected:
		class Block
			: public Control,
			public Attribute::Id,
			public Attribute::Pos,
			public Attribute::Size
		{
			Canvas* m_canvas;
		
		public:
			Block(Canvas* canvas)
				: m_canvas(canvas)
			{}
		
		private:
			void renderControl() override {
				m_canvas->setPos(m_pos);
				if (ImGui::BeginChild(getId().c_str(), m_size, true)) {
					renderBlock();
				}
				ImGui::EndChild();
			}

		protected:
			virtual void renderBlock() = 0;
		};
		
		ImDrawList* m_drawList;
		ImVec2 m_p0;
		ImVec2 m_p1;
		ImVec2 m_size;
		ImVec2 m_offset;
		ImVec2 m_origin;
		bool m_isGridRendered = true;
	
	public:
		Canvas()
		{}

	private:
		void renderControl() override {
			if (ImGui::BeginChild(getId().c_str(), m_size, false, ImGuiWindowFlags_NoScrollbar)) {
				ImGui::SetWindowFontScale(1.0f);
				m_drawList = ImGui::GetWindowDrawList();
				m_size = ImGui::GetContentRegionAvail();
				m_p0 = ImGui::GetCursorScreenPos();
				m_p1 = ImVec2(m_p0.x + m_size.x, m_p0.y + m_size.y);
				m_origin = ImVec2(m_p0.x + m_offset.x, m_p0.y + m_offset.y);

				ImGui::InvisibleButton(getId().c_str(), m_size, ImGuiButtonFlags_MouseButtonLeft);
				if (ImGui::IsItemActive())
				{
					if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
						const auto& io = ImGui::GetIO();
						m_offset.x += io.MouseDelta.x;
						m_offset.y += io.MouseDelta.y;
					}
				}

				ImGui::PushClipRect(m_p0, m_p1, true);
				renderCanvas();
				if (m_isGridRendered)
					renderGrid();
				ImGui::PopClipRect();
				ImGui::EndChild();
			}
		}

	protected:
		virtual void renderCanvas() = 0;

		void setPos(const ImVec2& pos) const {
			ImGui::SetCursorScreenPos(ImVec2(m_origin.x + pos.x, m_origin.y + pos.y));
		}

		void renderGrid() const {
			const float GRID_STEP = 64.0f;
			for (float x = fmodf(m_offset.x, GRID_STEP); x < m_size.x; x += GRID_STEP)
				m_drawList->AddLine(ImVec2(m_p0.x + x, m_p0.y), ImVec2(m_p0.x + x, m_p1.y), IM_COL32(200, 200, 200, 40));
			for (float y = fmodf(m_offset.y, GRID_STEP); y < m_size.y; y += GRID_STEP)
				m_drawList->AddLine(ImVec2(m_p0.x, m_p0.y + y), ImVec2(m_p1.x, m_p0.y + y), IM_COL32(200, 200, 200, 40));
		}
	};

	class FuncGraphViewerPanel : public AbstractPanel
	{
		class FuncGraphViewerCanvas : public Canvas
		{
			class CanvasPCodeBlock : public Block
			{
				FuncGraphViewerCanvas* m_canvas; //todo: remove
				const CE::Decompiler::PCodeBlock* m_pcodeBlock;
			public:
				CanvasPCodeBlock(FuncGraphViewerCanvas* canvas = nullptr, const CE::Decompiler::PCodeBlock* pcodeBlock = nullptr)
					: Block(canvas), m_canvas(canvas), m_pcodeBlock(pcodeBlock)
				{}

			private:
				void renderBlock() override {
					if (ImGui::BeginTable("table", 2, ImGuiTableFlags_Borders))
					{
						ImGui::TableNextColumn();
						Text::Text("ID = " + std::to_string(m_pcodeBlock->ID)).show();
						ImGui::TableNextColumn();
						Text::Text("level = " + std::to_string(m_pcodeBlock->m_level)).show();
						ImGui::EndTable();
					}
				}
			};
			
			FuncGraphViewerPanel* m_panel;
			std::map<const CE::Decompiler::PCodeBlock*, CanvasPCodeBlock*> m_canvasBlocks;
		public:
			FuncGraphViewerCanvas(FuncGraphViewerPanel* panel)
				: m_panel(panel)
			{
				const auto startPCodeBlock = m_panel->m_funcPCodeGraph->getStartBlock();
				std::map<const CE::Decompiler::PCodeBlock*, int> blockParentsCount;
				blockParentsCount[startPCodeBlock] = 0;
				buildFunctionGraph(startPCodeBlock, blockParentsCount);
			}

			~FuncGraphViewerCanvas() override {
				for (auto& [pcodeBlock, block] : m_canvasBlocks) {
					delete block;
				}
			}

		private:
			void renderCanvas() override {
				for(auto& [pcodeBlock, block] : m_canvasBlocks) {
					block->show();
				}
			}

			void buildFunctionGraph(const CE::Decompiler::PCodeBlock* pcodeBlock, std::map<const CE::Decompiler::PCodeBlock*, int>& blockParentsCount) {
				const auto parentsCount = pcodeBlock->getRefHighBlocksCount();
				if(blockParentsCount[pcodeBlock] == parentsCount) {
					// calculate position
					ImVec2 pos;
					pos.y = (pcodeBlock->m_level - 1) * 300.f;
					if(parentsCount == 1) {
						const auto parentBlock = *pcodeBlock->m_blocksReferencedTo.begin();
						pos.x = m_canvasBlocks[parentBlock]->getPos().x;
						if (auto parentNextBlocks = parentBlock->getNextBlocks(); parentNextBlocks.size() == 2) {
							const float ChildBlockOffset = 200.f;
							if (pcodeBlock == *parentNextBlocks.begin())
								pos.x += ChildBlockOffset;
							else pos.x -= ChildBlockOffset;
						}
					}
					else if (parentsCount > 1) {
						for(const auto parentBlock : pcodeBlock->m_blocksReferencedTo) {
							pos.x += m_canvasBlocks[parentBlock]->getPos().x;
						}
						pos.x /= parentsCount;
					}

					// create canvas block
					auto canvasBlock = new CanvasPCodeBlock(this, pcodeBlock);
					canvasBlock->getPos() = pos;
					canvasBlock->getSize() = ImVec2(200, 200);
					m_canvasBlocks[pcodeBlock] = canvasBlock;

					// go next pcode blocks
					for (auto nextPCodeBlock : pcodeBlock->getNextBlocks()) {
						if (pcodeBlock->m_level >= nextPCodeBlock->m_level)
							continue;
						if(blockParentsCount.find(nextPCodeBlock) == blockParentsCount.end())
							blockParentsCount[nextPCodeBlock] = 0;
						blockParentsCount[nextPCodeBlock] ++;
						buildFunctionGraph(nextPCodeBlock, blockParentsCount);
					}
				}
			}
		};
		
		CodeSectionController* m_codeSectionController;
		AbstractInstructionViewer* m_instructionViewer;
		CE::Decompiler::FunctionPCodeGraph* m_funcPCodeGraph = nullptr;
		FuncGraphViewerCanvas* m_funcGraphViewerCanvas = nullptr;
	public:
		FuncGraphViewerPanel(CodeSectionController* codeSectionController, AbstractInstructionViewer* instructionViewer)
			: AbstractPanel("Function graph viewer"), m_codeSectionController(codeSectionController), m_instructionViewer(instructionViewer)
		{}

		~FuncGraphViewerPanel() override {
			delete m_funcGraphViewerCanvas;
		}

		void setFuncGraph(CE::Decompiler::FunctionPCodeGraph* functionPCodeGraph) {
			m_funcPCodeGraph = functionPCodeGraph;
			delete m_funcGraphViewerCanvas;
			m_funcGraphViewerCanvas = new FuncGraphViewerCanvas(this);
		}

		StdWindow* createStdWindow() {
			return new StdWindow(this, ImGuiWindowFlags_NoScrollbar);
		}
	
	private:
		void renderPanel() override {
			m_funcGraphViewerCanvas->show();
		}
	};
};