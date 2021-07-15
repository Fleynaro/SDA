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
			bool m_isMouseDragging = false;
			float m_padding = 0.0f;
		public:
			Block(Canvas* canvas)
				: m_canvas(canvas)
			{}
		
		private:
			void renderControl() override {
				ImGui::SetCursorScreenPos(m_canvas->toAbsPos(m_pos));
				ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32_BLACK);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(m_padding, m_padding));
				ImGui::BeginChild(getId().c_str(), m_size * m_canvas->m_scaling, true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
				{
					if (ImGui::IsWindowHovered() || m_isMouseDragging) {
						if ((m_isMouseDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left))) {
							m_pos += ImGui::GetIO().MouseDelta / m_canvas->m_scaling;
						}
					}
					
					ImGui::BeginGroup();
					{
						renderBlock();
					}
					ImGui::EndGroup();
					if (m_size.x == 0.0f || m_size.y == 0.0f) {
						const auto calcMinSize = ImGui::GetItemRectSize();
						if (m_size.x == 0.0f)
							m_size.x = calcMinSize.x;
						if (m_size.y == 0.0f)
							m_size.y = calcMinSize.y;
					}
				}
				ImGui::EndChild();
				ImGui::PopStyleVar();
				ImGui::PopStyleColor();
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
		float m_scaling = 1.0f;
		bool m_scalingEnabled = true;
		bool m_scalingEnabledWhenBlocksHovered = true; // false if blocks have scrollbar
		bool m_isGridRendered = true;
	
	public:
		Canvas()
		{}

	private:
		void renderControl() override {
			m_scalingEnabled = true;
			ImGui::BeginChild(getId().c_str(), m_size, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			{
				ImGui::SetWindowFontScale(m_scaling);
				m_drawList = ImGui::GetWindowDrawList();
				m_size = ImGui::GetContentRegionAvail();
				m_p0 = ImGui::GetCursorScreenPos();
				m_p1 = m_p0 + m_size;
				m_origin = m_p0 + m_offset;

				ImGui::InvisibleButton("##empty", m_size, ImGuiButtonFlags_MouseButtonLeft);
				if (ImGui::IsItemActive()) {
					if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
						m_offset += ImGui::GetIO().MouseDelta;
					}
				}
				if (m_scalingEnabled) {
					if (ImGui::IsWindowHovered()) {
						if (m_scalingEnabledWhenBlocksHovered || ImGui::IsItemHovered()) {
							m_scaling += ImGui::GetIO().MouseWheel * 0.1f;
							m_scaling = std::min(2.0f, m_scaling);
							m_scaling = std::max(0.2f, m_scaling);
						}
					}
				}

				ImGui::PushClipRect(m_p0, m_p1, true);
				if (m_isGridRendered)
					renderGrid();
				renderCanvas();
				ImGui::PopClipRect();
			}
			ImGui::EndChild();
		}

	protected:
		virtual void renderCanvas() = 0;

		ImVec2 toAbsPos(const ImVec2& pos) const {
			return m_origin + pos * m_scaling;
		}

		void renderGrid() const {
			const float GRID_STEP = 64.0f * m_scaling;
			for (float x = fmodf(m_offset.x, GRID_STEP); x < m_size.x; x += GRID_STEP)
				m_drawList->AddLine(ImVec2(m_p0.x + x, m_p0.y), ImVec2(m_p0.x + x, m_p1.y), IM_COL32(200, 200, 200, 40), m_scaling);
			for (float y = fmodf(m_offset.y, GRID_STEP); y < m_size.y; y += GRID_STEP)
				m_drawList->AddLine(ImVec2(m_p0.x, m_p0.y + y), ImVec2(m_p1.x, m_p0.y + y), IM_COL32(200, 200, 200, 40), m_scaling);
		}
	};

	class FuncGraphViewerPanel : public AbstractPanel
	{
		/*
		 * todo:
		 * 1) loop line
		 * 2) drag and drop block
		 * 3) scale
		 * 4) autosize
		 */
		class FuncGraphViewerCanvas : public Canvas
		{
			class CanvasPCodeBlock : public Block
			{
				class InstructioViewer : public AbstractInstructionViewer
				{
				public:
					using AbstractInstructionViewer::AbstractInstructionViewer;

				protected:
					void renderMnemonic() override {
						ImGui::TableNextColumn();
						AbstractInstructionViewer::renderMnemonic();
						SameLine(2.0f);
					}

					void renderOperands() override {
						AbstractInstructionViewer::renderOperands();
					}
				};
				
				FuncGraphViewerCanvas* m_canvas; //todo: remove
			public:
				const CE::Decompiler::PCodeBlock* m_pcodeBlock;
				
				CanvasPCodeBlock(FuncGraphViewerCanvas* canvas = nullptr, const CE::Decompiler::PCodeBlock* pcodeBlock = nullptr)
					: Block(canvas), m_canvas(canvas), m_pcodeBlock(pcodeBlock)
				{}

			private:
				void renderBlock() override {
					Text::Text("ID = " + std::to_string(m_pcodeBlock->ID) + ", level = " + std::to_string(m_pcodeBlock->m_level)).show();
					
					if (ImGui::BeginTable("table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
					{
						/*ImGui::TableSetupColumn("Offset");
						ImGui::TableSetupColumn("Instruction");
						ImGui::TableHeadersRow();*/
						
						const auto graphOffset = m_pcodeBlock->m_funcPCodeGraph->getStartBlock()->getMinOffset().getByteOffset();
						auto offset = m_pcodeBlock->getMinOffset().getByteOffset();
						while(offset < m_pcodeBlock->getMaxOffset().getByteOffset()) {
							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							using namespace Helper::String;
							Text::Text("+" + NumberToHex(offset - graphOffset)).show();
							
							InstructionViewInfo instrViewInfo;
							auto image = m_canvas->m_panel->m_imageDec->getImage();
							m_canvas->m_panel->m_instructionViewDecoder->decode(image->getData() + image->toImageOffset(offset), &instrViewInfo);
							InstructioViewer instructionViewer(&instrViewInfo);
							instructionViewer.show();
							offset += instrViewInfo.m_length;
						}
						ImGui::EndTable();
					}
				}	
			};
			
			FuncGraphViewerPanel* m_panel;
			std::map<const CE::Decompiler::PCodeBlock*, CanvasPCodeBlock*> m_canvasBlocks;
			bool isFirstRender = true;
		public:
			FuncGraphViewerCanvas(FuncGraphViewerPanel* panel)
				: m_panel(panel)
			{
				// create canvas blocks
				for(auto pcodeBlock : m_panel->m_funcPCodeGraph->getBlocks()) {
					m_canvasBlocks[pcodeBlock] = new CanvasPCodeBlock(this, pcodeBlock);
				}
			}

			~FuncGraphViewerCanvas() override {
				for (auto& [pcodeBlock, block] : m_canvasBlocks) {
					delete block;
				}
			}

		private:
			void renderCanvas() override {
				if(isFirstRender) {
					// render canvas blocks to calculate its sizes
					for (auto& [pcodeBlock, canvasBlock] : m_canvasBlocks)
						canvasBlock->show();
					arrangeCanvasBlocks();
					isFirstRender = false;
					return;
				}
				
				for(auto& [pcodeBlock, canvasBlock] : m_canvasBlocks) {
					canvasBlock->show();
					
					// render lines
					for (auto nextPCodeBlock : pcodeBlock->getNextBlocks()) {
						const auto nextCanvasBlock = m_canvasBlocks[nextPCodeBlock];
						drawJmpLine(canvasBlock, nextCanvasBlock);
					}
				}
			}

			void arrangeCanvasBlocks() {
				// calculate coordinate X for canvas blocks
				const auto startPCodeBlock = m_panel->m_funcPCodeGraph->getStartBlock();
				std::map<const CE::Decompiler::PCodeBlock*, int> blockParentsCount;
				blockParentsCount[startPCodeBlock] = 0;
				buildFunctionGraph(startPCodeBlock, blockParentsCount);
				
				// calculate coordinate Y for canvas blocks
				std::map<int, std::list<CanvasPCodeBlock*>> canvasBlocks;
				for (auto& [pcodeBlock, canvasBlock] : m_canvasBlocks) {
					canvasBlocks[pcodeBlock->m_level].push_back(canvasBlock);
				}
				float posY = 0.0f;
				for(auto& [level, canvasBlocks] : canvasBlocks) {
					for (auto canvasBlock : canvasBlocks)
						canvasBlock->getPos().y = posY;
					// calculate max size
					float maxSizeY = 0.0f;
					for (auto canvasBlock : canvasBlocks)
						maxSizeY = std::max(maxSizeY, canvasBlock->getSize().y);
					posY += maxSizeY + 100.0f;
				}
			}

			void drawJmpLine(CanvasPCodeBlock* fromBlock, CanvasPCodeBlock* toBlock) const {
				const bool isUncondJmp = fromBlock->m_pcodeBlock->getNextNearBlock() == nullptr;
				const bool isFar = fromBlock->m_pcodeBlock->getNextFarBlock() == toBlock->m_pcodeBlock;
				const bool isLoop = fromBlock->m_pcodeBlock->m_level >= toBlock->m_pcodeBlock->m_level;
				const auto color = ToImGuiColorU32(isLoop ? 0xfeffc7FF : (isFar ? 0xbbe3faFF : 0xfabbbbFF));
				
				// line
				const auto bPos1 = fromBlock->getPos();
				const auto bPos2 = toBlock->getPos();
				const auto bSize1 = fromBlock->getSize();
				const auto bSize2 = toBlock->getSize();
				const auto p1 = ImVec2(bPos1.x + bSize1.x * (0.4f + isFar * 0.2f - isUncondJmp * 0.1f), bPos1.y + bSize1.y);
				const auto p2 = ImVec2(bPos2.x + bSize2.x / 2.0f, bPos2.y);
				m_drawList->AddLine(toAbsPos(p1), toAbsPos(p2), color, 2.0f * m_scaling);
				
				// label
				const auto labelPos = p1 + ImVec2(-1.0f, 0.0f);
				const auto label = isFar ? "far" : "near";
				m_drawList->AddText(toAbsPos(labelPos), color, label);

				// loop label
				if(isLoop) {
					const auto loopLabelPos = (p1 + p2) / 2.0f;
					m_drawList->AddText(toAbsPos(loopLabelPos), color, "loop");
				}
			}

			void buildFunctionGraph(const CE::Decompiler::PCodeBlock* pcodeBlock, std::map<const CE::Decompiler::PCodeBlock*, int>& blockParentsCount) {
				auto parentBlocks = pcodeBlock->getRefHighBlocks();
				if(blockParentsCount[pcodeBlock] == parentBlocks.size()) {
					// calculate coordinate X
					float posX = 0.0f;
					if(parentBlocks.size() == 1) {
						const auto parentBlock = *parentBlocks.begin();
						posX = m_canvasBlocks[parentBlock]->getPos().x;
						if (parentBlock->getNextBlocks().size() == 2) {
							if (pcodeBlock == parentBlock->getNextFarBlock())
								posX += m_canvasBlocks[parentBlock]->getSize().x;
							else posX -= m_canvasBlocks[pcodeBlock]->getSize().x;
						} else {
							posX += (m_canvasBlocks[parentBlock]->getSize().x - m_canvasBlocks[pcodeBlock]->getSize().x) / 2.0f;
						}
					}
					else if (parentBlocks.size() > 1) {
						for(const auto parentBlock : pcodeBlock->m_blocksReferencedTo) {
							posX += m_canvasBlocks[parentBlock]->getPos().x;
						}
						posX /= parentBlocks.size();
					}
					m_canvasBlocks[pcodeBlock]->getPos().x = posX;

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

		const CE::ImageDecorator* m_imageDec;
		AbstractInstructionViewDecoder* m_instructionViewDecoder;
		CE::Decompiler::FunctionPCodeGraph* m_funcPCodeGraph = nullptr;
		FuncGraphViewerCanvas* m_funcGraphViewerCanvas = nullptr;
	public:
		FuncGraphViewerPanel(const CE::ImageDecorator* imageDec, AbstractInstructionViewDecoder* instructionViewDecoder)
			: AbstractPanel("Function graph viewer"), m_imageDec(imageDec), m_instructionViewDecoder(instructionViewDecoder)
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