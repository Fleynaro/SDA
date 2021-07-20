#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "InstructionViewer.h"
#include "decompiler/SDA/SdaCodeGraph.h"
#include "decompiler/DecCodeGenerator.h"
#include "decompiler/DecMisc.h"
#include <imgui_wrapper/controls/Control.h>
#include <bitset>

namespace GUI
{
	class DecompiledCodeViewer
		: public Control,
		public Attribute::Id
	{
		friend class DecompiledCodeViewerPanel;
	protected:
		class CodeGenerator : public CE::Decompiler::CodeViewGenerator
		{
			inline const static ColorRGBA COLOR_DEBUG_INFO = 0xd982c2FF;
			inline const static ColorRGBA COLOR_BG_TEXT_ON_HOVER = 0x212e40FF;
			inline const static ColorRGBA COLOR_BG_SELECTED_TEXT = 0x374559FF;
			inline const static ColorRGBA COLOR_FRAME_SELECTED_TEXT = 0x8c8c8cFF;
			inline const static ColorRGBA COLOR_DATA_TYPE = 0xd9c691FF;
			inline const static ColorRGBA COLOR_NUMBER = 0xe6e6e6FF;

			static void HighlightItem(ColorRGBA color) {
				ImGui::GetWindowDrawList()->AddRectFilled(
					ImGui::GetItemRectMin() - ImVec2(2.0f, 1.0f), ImGui::GetItemRectMax() + ImVec2(2.0f, 1.0f), ToImGuiColorU32(color));
				ImGui::SetCursorScreenPos(ImGui::GetItemRectMin());
			}

			static void FrameItem(ColorRGBA color) {
				ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin() - ImVec2(1.0f, 1.0f), ImGui::GetItemRectMax() + ImVec2(1.0f, 1.0f), ToImGuiColorU32(color));
			}

			class ExprTreeGenerator : public CE::Decompiler::ExprTree::ExprTreeViewGenerator
			{
				CodeGenerator* m_parentGen;
			public:
				ExprTreeGenerator(CodeGenerator* parentGen)
					: m_parentGen(parentGen)
				{}

			private:
				void generateToken(const std::string& text, TokenType tokenType) override {
					ColorRGBA color = 0xe6e6e6FF;
					if (tokenType == TOKEN_DATA_TYPE)
						color = COLOR_DATA_TYPE;
					else if (tokenType == TOKEN_DEC_SYMBOL)
						color = 0xb7c2b2FF;
					else if (tokenType == TOKEN_SDA_SYMBOL)
						color = 0xcde0c5FF;
					else if (tokenType == TOKEN_FUNCTION_CALL)
						color = 0xdedac5FF;
					else if (tokenType == TOKEN_DEBUG_INFO)
						color = COLOR_DEBUG_INFO;
					Text::ColoredText(text, color).show();
					SameLine(1.0f);
				}

				void generateSdaSymbol(CE::Symbol::ISymbol* symbol) override {
					ExprTreeViewGenerator::generateSdaSymbol(symbol);
					auto& selSymbols = m_parentGen->m_decCodeViewer->m_selectedSymbols;
					if (ImGui::IsItemClicked()) {
						selSymbols[symbol] = 1000;
					}

					const auto hasSymbolSelected = selSymbols.find(symbol) != selSymbols.end();
					if (ImGui::IsItemHovered() || hasSymbolSelected) {
						HighlightItem(ImGui::IsItemHovered() ? COLOR_BG_TEXT_ON_HOVER : COLOR_BG_SELECTED_TEXT);
						ExprTreeViewGenerator::generateSdaSymbol(symbol);
						if (hasSymbolSelected)
							selSymbols[symbol]++;
					}

					if (ImGui::IsItemHovered()) {
						ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
						ImGui::SetTooltip(("Data type: " + symbol->getDataType()->getDisplayName()).c_str());
					}
				}

				void generateNumberLeaf(CE::Decompiler::NumberLeaf* leaf) override {
					ExprTreeViewGenerator::generateNumberLeaf(leaf);
					if (ImGui::IsItemHovered()) {
						HighlightItem(COLOR_BG_TEXT_ON_HOVER);
						ExprTreeViewGenerator::generateNumberLeaf(leaf);
						ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
						RenderNumberPresentations(leaf->getValue());
					}
				}
				
				void generateSdaNumberLeaf(CE::Decompiler::SdaNumberLeaf* leaf) override {
					using namespace Helper::String;
					ExprTreeViewGenerator::generateSdaNumberLeaf(leaf);
					if (ImGui::IsItemHovered()) {
						HighlightItem(COLOR_BG_TEXT_ON_HOVER);
						ExprTreeViewGenerator::generateSdaNumberLeaf(leaf);
						ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
						RenderNumberPresentations(leaf->getValue());
					}
				}

				static void RenderNumberPresentations(uint64_t value) {
					using namespace Helper::String;
					ImGui::BeginTooltip();
					if(ImGui::BeginTable("number presentation table", 2)) {
						const auto table = {
							std::pair("hex", NumberToHex(value)),
							std::pair("binary", std::bitset<64>(value).to_string()),
							std::pair("uint64_t", std::to_string(value)),
							std::pair("int64_t", std::to_string(reinterpret_cast<int64_t&>(value))),
							std::pair("float", std::to_string(reinterpret_cast<float&>(value))),
							std::pair("double", std::to_string(reinterpret_cast<double&>(value))),
						};
						for(const auto& [type, value] : table) {
							ImGui::TableNextColumn();
							Text::ColoredText(type, COLOR_DATA_TYPE).show();
							ImGui::TableNextColumn();
							Text::ColoredText(value, COLOR_NUMBER).show();
						}
						ImGui::EndTable();
					}
					ImGui::EndTooltip();
				}

				void generateRoundBracket(const std::string& text, CE::Decompiler::INode* node) override {
					if (text == "(") {
						ImGui::BeginGroup();
					}
					
					ExprTreeViewGenerator::generateRoundBracket(text, node);
					if(node == m_parentGen->m_decCodeViewer->m_selectedNode) {
						FrameItem(COLOR_FRAME_SELECTED_TEXT);
					}

					if (text == ")") {
						ImGui::EndGroup();
						SameLine(0.0f);
						if (!m_parentGen->m_hasNodeSelected) {
							if (ImGui::IsItemClicked()) {
								m_parentGen->m_decCodeViewer->m_selectedNode = node;
								m_parentGen->m_hasNodeSelected = true;
							}
						}
					}
				}
			};

			DecompiledCodeViewer* m_decCodeViewer;
			ExprTreeGenerator m_exprTreeGenerator;
			bool m_hasNodeSelected = false;
			bool m_hasBlockListSelected = false;
		public:
			CodeGenerator(DecompiledCodeViewer* decompiledCodeViewer)
				: CodeViewGenerator(&m_exprTreeGenerator), m_decCodeViewer(decompiledCodeViewer), m_exprTreeGenerator(this)
			{
				setMinInfoToShow();
				m_SHOW_LINEAR_LEVEL_EXT = true;
			}

		private:
			void generateToken(const std::string& text, TokenType tokenType) override {
				if (tokenType == TOKEN_END_LINE) {
					NewLine();
					return;
				}

				ColorRGBA color = 0xe6e6e6FF;
				if (tokenType == TOKEN_OPERATOR)
					color = 0x5d9ad4FF;
				else if (tokenType == TOKEN_COMMENT)
					color = 0x79ba8dFF;
				else if (tokenType == TOKEN_LABEL)
					color = 0x51e066FF;
				else if (tokenType == TOKEN_DEBUG_INFO)
					color = COLOR_DEBUG_INFO;
				Text::ColoredText(text, color).show();
				SameLine(1.0f);
			}

			void generateBlockList(CE::Decompiler::LinearView::BlockList* blockList, bool generatingTabs, bool generatingBraces) override {
				if (m_decCodeViewer->m_hidedBlockLists.find(blockList) != m_decCodeViewer->m_hidedBlockLists.end()) {
					ImGui::BeginGroup();
					generateTabs();
					generateToken("{", TOKEN_CURLY_BRACKET);
					generateToken("...", TOKEN_OTHER);
					generateToken("}", TOKEN_CURLY_BRACKET);
					ImGui::EndGroup();
					if (ImGui::IsItemHovered()) {
						ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
						ImGui::SetTooltip("Open");
					}
					if (ImGui::IsItemClicked()) {
						m_decCodeViewer->m_hidedBlockLists.erase(blockList);
					}
					return;
				}
				
				CodeViewGenerator::generateBlockList(blockList, generatingTabs, generatingBraces);
			}

			void generateCode(CE::Decompiler::DecBlock* decBlock) override {
				// show block with assembler code
				if(m_decCodeViewer->m_showAsm) {
					const auto pcodeBlock = decBlock->m_pcodeBlock;
					generateTabs();
					if (ImGui::BeginTable(("table-" + pcodeBlock->getName()).c_str(), 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
					{
						const auto graphOffset = pcodeBlock->m_funcPCodeGraph->getStartBlock()->getMinOffset().getByteOffset();
						auto offset = pcodeBlock->getMinOffset().getByteOffset();
						while (offset < pcodeBlock->getMaxOffset().getByteOffset()) {
							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							using namespace Helper::String;
							Text::Text("+" + NumberToHex(offset - graphOffset)).show();

							InstructionViewInfo instrViewInfo;
							const auto image = m_decCodeViewer->m_image;
							m_decCodeViewer->m_instructionViewDecoder->decode(image->getData() + image->toImageOffset(offset), &instrViewInfo);
							InstructionTableRowViewer2 instructionViewer(&instrViewInfo);
							instructionViewer.show();

							offset += instrViewInfo.m_length;
						}
						ImGui::EndTable();
					}
				}
				CodeViewGenerator::generateCode(decBlock);
			}

			void generateCurlyBracket(const std::string& text, CE::Decompiler::LinearView::BlockList* blockList) override {
				if (text == "{") {
					ImGui::BeginGroup();
				}
				
				CodeViewGenerator::generateCurlyBracket(text, blockList);
				if (blockList == m_decCodeViewer->m_selectedBlockList) {
					FrameItem(COLOR_FRAME_SELECTED_TEXT);
				}
				
				if (text == "}") {
					ImGui::EndGroup();
					if (!m_hasBlockListSelected) {
						if (ImGui::IsItemClicked()) {
							m_decCodeViewer->m_selectedBlockList = blockList;
							m_hasBlockListSelected = true;
						}
						if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
							m_decCodeViewer->m_hidedBlockLists.insert(blockList);
							m_hasBlockListSelected = true;
						}
					}
				}
			}
		};

		CE::Decompiler::LinearView::BlockList* m_blockList;
		std::map<CE::Symbol::ISymbol*, int> m_selectedSymbols; // todo: create a controller?
		CE::Decompiler::INode* m_selectedNode = nullptr;
		CE::Decompiler::LinearView::BlockList* m_selectedBlockList = nullptr;
		std::set<CE::Decompiler::LinearView::BlockList*> m_hidedBlockLists;
		CE::AbstractImage* m_image = nullptr;
		AbstractInstructionViewDecoder* m_instructionViewDecoder = nullptr;
	public:
		bool m_showAsm = false;
		
		DecompiledCodeViewer(CE::Decompiler::LinearView::BlockList* blockList)
			: m_blockList(blockList)
		{}

		~DecompiledCodeViewer() {
			delete m_instructionViewDecoder;
		}

		void setInfoToShowAsm(CE::AbstractImage* image, AbstractInstructionViewDecoder* instructionViewDecoder) {
			m_image = image;
			m_instructionViewDecoder = instructionViewDecoder;
		}

	protected:
		virtual void renderCode() {
			CodeGenerator generator(this);
			generator.generate(m_blockList);
		}

	private:
		void renderControl() override {
			ImGui::BeginChild(getId().c_str());
			if (ImGui::IsWindowHovered()) {
				if (ImGui::IsMouseClicked(0)) {
					m_selectedSymbols.clear();
					m_selectedNode = nullptr;
					m_selectedBlockList = nullptr;
				}
			}
			removeSingleSelectedSymbols();
			
			renderCode();
			ImGui::EndChild();
		}

		void removeSingleSelectedSymbols() {
			auto it = m_selectedSymbols.begin();
			while (it != m_selectedSymbols.end()) {
				auto& leafsCount = it->second;
				if (leafsCount == 1) {
					it = m_selectedSymbols.erase(it);
				}
				else {
					leafsCount = 0;
					++it;
				}
			}
		}
	};
	
	class DecompiledCodeViewerPanel : public AbstractPanel
	{
		class DecompiledCodeViewerWithHeader : public DecompiledCodeViewer
		{
			CE::Decompiler::SdaCodeGraph* m_sdaCodeGraph;
		public:
			DecompiledCodeViewerWithHeader(CE::Decompiler::LinearView::BlockList* blockList, CE::Decompiler::SdaCodeGraph* sdaCodeGraph)
				: DecompiledCodeViewer(blockList), m_sdaCodeGraph(sdaCodeGraph)
			{}

		private:
			void renderCode() override {
				CodeGenerator generator(this);
				generator.generateHeader(m_sdaCodeGraph);
				generator.generateEndLine();
				DecompiledCodeViewer::renderCode();
			}
		};
		
		CE::Decompiler::LinearView::BlockList* m_blockList;
	public:
		DecompiledCodeViewer* m_decompiledCodeViewer;
		
		DecompiledCodeViewerPanel(CE::Decompiler::LinearView::BlockList* blockList)
			: AbstractPanel("Decompiled code viewer"), m_blockList(blockList)
		{}
		
		DecompiledCodeViewerPanel(CE::Decompiler::SdaCodeGraph* sdaCodeGraph)
			: DecompiledCodeViewerPanel(CE::Decompiler::Misc::BuildBlockList(sdaCodeGraph->getDecGraph()))
		{
			m_decompiledCodeViewer = new DecompiledCodeViewerWithHeader(m_blockList, sdaCodeGraph);
		}

		DecompiledCodeViewerPanel(CE::Decompiler::DecompiledCodeGraph* decCodeGraph)
			: DecompiledCodeViewerPanel(CE::Decompiler::Misc::BuildBlockList(decCodeGraph))
		{
			m_decompiledCodeViewer = new DecompiledCodeViewer(m_blockList);
		}

		~DecompiledCodeViewerPanel() override {
			delete m_blockList;
			delete m_decompiledCodeViewer;
		}

		StdWindow* createStdWindow() {
			return new StdWindow(this, ImGuiWindowFlags_MenuBar);
		}

	private:
		void renderPanel() override {
			m_decompiledCodeViewer->show();
		}

		void renderMenuBar() override {
			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Show all blocks")) {
					m_decompiledCodeViewer->m_hidedBlockLists.clear();
				}
				
				if (ImGui::MenuItem("Show assembler", nullptr, m_decompiledCodeViewer->m_showAsm)) {
					m_decompiledCodeViewer->m_showAsm ^= true;
				}
				ImGui::EndMenu();
			}
		}
	};
};