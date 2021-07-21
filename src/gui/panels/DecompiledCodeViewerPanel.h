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
					else if (tokenType == TOKEN_FUNCTION_CALL)
						color = 0xdedac5FF;
					else if (tokenType == TOKEN_DEBUG_INFO)
						color = COLOR_DEBUG_INFO;
					Text::ColoredText(text, color).show();
					SameLine(1.0f);
				}

				void generateSdaSymbol(CE::Symbol::ISymbol* symbol) override {
					RenderSdaSymbol(symbol);
					auto& selSymbols = m_parentGen->m_decCodeViewer->m_selectedSymbols;
					if (ImGui::IsItemClicked()) {
						selSymbols[symbol] = 1000;
					}

					const auto hasSymbolSelected = selSymbols.find(symbol) != selSymbols.end();
					if (ImGui::IsItemHovered() || hasSymbolSelected) {
						HighlightItem(ImGui::IsItemHovered() ? COLOR_BG_TEXT_ON_HOVER : COLOR_BG_SELECTED_TEXT);
						RenderSdaSymbol(symbol);
						if (hasSymbolSelected)
							selSymbols[symbol]++;
					}

					if (ImGui::IsItemHovered()) {
						ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
						ImGui::SetTooltip(("Data type: " + symbol->getDataType()->getDisplayName()).c_str());
					}
				}

				static void RenderSdaSymbol(CE::Symbol::ISymbol* symbol) {
					ColorRGBA color = 0x0;
					if (symbol->getType() == CE::Symbol::FUNC_PARAMETER)
						color = 0xcf7e7eFF;
					else if (symbol->getType() == CE::Symbol::GLOBAL_VAR)
						color = 0x6784c7FF;
					else if (symbol->getType() == CE::Symbol::LOCAL_INSTR_VAR)
						color = 0x8dbfccFF;
					else if (symbol->getType() == CE::Symbol::LOCAL_STACK_VAR)
						color = 0xbaf5edFF;
					else if (symbol->getType() == CE::Symbol::FUNCTION)
						color = 0xcacc9fFF;
					else if (symbol->getType() == CE::Symbol::STRUCT_FIELD)
						color = 0xdbdbdbFF;
					Text::ColoredText(symbol->getName(), color).show();
					SameLine(1.0f);
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

				void generateRoundBracket(const std::string& text, void* obj) override {
					if (text == "(") {
						ImGui::BeginGroup();
					}
					
					ExprTreeViewGenerator::generateRoundBracket(text, obj);
					if(obj == m_parentGen->m_decCodeViewer->m_selectedObj) {
						FrameItem(COLOR_FRAME_SELECTED_TEXT);
					}

					if (text == ")") {
						ImGui::EndGroup();
						SameLine(0.0f);
						if (!m_parentGen->m_hasNodeSelected) {
							if (ImGui::IsItemClicked()) {
								m_parentGen->m_decCodeViewer->m_selectedObj = obj;
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
				using namespace CE::Decompiler;
				const auto pcodeBlock = decBlock->m_pcodeBlock;
				
				// show block with assembler code & PCode
				if(m_decCodeViewer->m_show.Asm) {
					generateTabs();
					if (ImGui::BeginTable(("table1-" + pcodeBlock->getName()).c_str(), 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
					{
						RenderAsmListing(pcodeBlock, m_decCodeViewer->m_image, m_decCodeViewer->m_instructionViewDecoder, m_decCodeViewer->m_show.PCode);
						ImGui::EndTable();
					}
				}
				
				CodeViewGenerator::generateCode(decBlock);
				
				// show states of execution contexts
				if (m_decCodeViewer->m_show.ExecCtxs) {
					auto execCtx = m_decCodeViewer->m_primaryDecompiler->m_decompiledBlocks[pcodeBlock].m_execCtx;
					generateTabs();
					if (ImGui::BeginTable(("table2-" + pcodeBlock->getName()).c_str(), 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
					{
						ImGui::TableSetupColumn("Register");
						ImGui::TableSetupColumn("Using");
						ImGui::TableSetupColumn("Expression");
						ImGui::TableHeadersRow();
						for(const auto& [regId, registers] : execCtx->m_registerExecCtx.m_registers) {
							for (const auto regInfo : registers) {
								ImGui::TableNextRow();
								
								ImGui::TableNextColumn();
								const auto regName = InstructionViewGenerator::GenerateRegisterName(regInfo.m_register);
								Text::Text(regName).show();
								
								ImGui::TableNextColumn();
								std::string usingType = "not";
								if (regInfo.m_using == RegisterExecContext::RegisterInfo::REGISTER_FULLY_USING)
									usingType = "full";
								else if (regInfo.m_using == RegisterExecContext::RegisterInfo::REGISTER_PARTIALLY_USING)
									usingType = "part";
								Text::Text(usingType).show();
								
								ImGui::TableNextColumn();
								m_exprTreeGenerator.generate(regInfo.m_expr->getNode());
							}
						}
						ImGui::EndTable();
					}
				}
			}

			void generateCurlyBracket(const std::string& text, CE::Decompiler::LinearView::BlockList* blockList) override {
				const auto isSelected = blockList == m_decCodeViewer->m_selectedBlockList;
				if (text == "{") {
					generateToken(text, TOKEN_CURLY_BRACKET);
					if (isSelected) {
						FrameItem(COLOR_FRAME_SELECTED_TEXT);
					}
					curlyBracketEvent(blockList);
					generateEndLine();
					ImGui::BeginGroup();
				}
				
				if (text == "}") {
					generateToken(text, TOKEN_CURLY_BRACKET);
					if (isSelected) {
						FrameItem(COLOR_FRAME_SELECTED_TEXT);
					}
					ImGui::EndGroup();
					curlyBracketEvent(blockList);
				}
			}

			void curlyBracketEvent(CE::Decompiler::LinearView::BlockList* blockList) {
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
		};

		CE::Decompiler::LinearView::BlockList* m_blockList;
		std::map<CE::Symbol::ISymbol*, int> m_selectedSymbols; // todo: create a controller?
		void* m_selectedObj = nullptr;
		CE::Decompiler::LinearView::BlockList* m_selectedBlockList = nullptr;
		std::set<CE::Decompiler::LinearView::BlockList*> m_hidedBlockLists;
		CE::AbstractImage* m_image = nullptr;
		AbstractInstructionViewDecoder* m_instructionViewDecoder = nullptr;
		CE::Decompiler::PrimaryDecompiler* m_primaryDecompiler = nullptr;
	public:
		struct
		{
			bool Asm = false;
			bool PCode = false;
			bool ExecCtxs = false;
			bool DebugComments = true;
		} m_show;
		
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

		void setInfoToShowExecCtxs(CE::Decompiler::PrimaryDecompiler* primaryDecompiler) {
			m_primaryDecompiler = primaryDecompiler;
		}

	protected:
		virtual void renderCode() {
			CodeGenerator generator(this);
			generator.m_SHOW_ALL_COMMENTS = m_show.DebugComments;
			generator.generate(m_blockList);
		}

	private:
		void renderControl() override {
			ImGui::BeginChild(getId().c_str());
			if (ImGui::IsWindowHovered()) {
				if (ImGui::IsMouseClicked(0)) {
					m_selectedSymbols.clear();
					m_selectedObj = nullptr;
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
				generator.m_SHOW_ALL_COMMENTS = m_show.DebugComments;
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

				ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
				if (ImGui::MenuItem("Show assembler", nullptr, m_decompiledCodeViewer->m_show.Asm)) {
					m_decompiledCodeViewer->m_show.Asm ^= true;
				}
				if (m_decompiledCodeViewer->m_show.Asm) {
					if (ImGui::MenuItem("Show PCode", nullptr, m_decompiledCodeViewer->m_show.PCode)) {
						m_decompiledCodeViewer->m_show.PCode ^= true;
					}
				}
				if (ImGui::MenuItem("Show exec. contexts", nullptr, m_decompiledCodeViewer->m_show.ExecCtxs)) {
					m_decompiledCodeViewer->m_show.ExecCtxs ^= true;
				}
				if (ImGui::MenuItem("Show comments", nullptr, m_decompiledCodeViewer->m_show.DebugComments)) {
					m_decompiledCodeViewer->m_show.DebugComments ^= true;
				}
				ImGui::PopItemFlag();
				ImGui::EndMenu();
			}
		}
	};
};