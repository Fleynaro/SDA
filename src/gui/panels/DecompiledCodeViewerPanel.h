#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "DataTypeManagerPanel.h"
#include "imgui_internal.h"
#include "InstructionViewer.h"
#include "controllers/SymbolManagerController.h"
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
		class AbstractContextPanel : public AbstractPanel
		{
		protected:
			DecompiledCodeViewer* m_decCodeViewer;
			ImVec2 m_winPos;
		
			AbstractContextPanel(DecompiledCodeViewer* decCodeViewer, ImVec2 winPos = ImVec2())
				: m_decCodeViewer(decCodeViewer), m_winPos(winPos)
			{}
		};
		
		class BlockListContextPanel : public AbstractContextPanel
		{
			CE::Decompiler::LinearView::BlockList* m_blockList;
		public:
			BlockListContextPanel(CE::Decompiler::LinearView::BlockList* blockList, DecompiledCodeViewer* decCodeViewer)
				: AbstractContextPanel(decCodeViewer), m_blockList(blockList)
			{}

		private:
			void renderPanel() override {
				if (ImGui::MenuItem("Hide block")) {
					m_decCodeViewer->m_hidedBlockLists.insert(m_blockList);
				}
			}
		};
		
		class SymbolContextPanel : public AbstractContextPanel
		{
			CE::Symbol::ISymbol* m_symbol;
			SymbolManagerController m_symbolManagerController;
		public:
			SymbolContextPanel(CE::Symbol::ISymbol* symbol, DecompiledCodeViewer* decCodeViewer, ImVec2 winPos)
				: AbstractContextPanel(decCodeViewer, winPos), m_symbolManagerController(symbol->getManager()), m_symbol(symbol)
			{}

		private:
			void renderPanel() override {
				auto& builtinWin = m_decCodeViewer->m_builtinWindow;
				if (ImGui::MenuItem("Rename")) {
					delete builtinWin;
					const auto panel = new BuiltinTextInputPanel(m_symbol->getName());
					panel->handler([&](const std::string& name)
						{
							if (const auto dbSymbol = dynamic_cast<CE::Symbol::AbstractSymbol*>(m_symbol)) {
								dbSymbol->setName(name);
								dbSymbol->getManager()->getProject()->getTransaction()->markAsDirty(dbSymbol);
							}
							m_decCodeViewer->m_codeChanged = true;
						});
					builtinWin = new PopupBuiltinWindow(panel);
					builtinWin->getPos() = m_winPos;
					builtinWin->open();
				}
				if (ImGui::MenuItem("Change data type")) {
					delete builtinWin;
					const auto panel = new DataTypeSelectorPanel(m_symbol->getManager()->getProject()->getTypeManager());
					panel->handler([&](CE::DataTypePtr dataType)
						{
							if (const auto dbSymbol = dynamic_cast<CE::Symbol::AbstractSymbol*>(m_symbol)) {
								dbSymbol->setDataType(dataType);
								dbSymbol->getManager()->getProject()->getTransaction()->markAsDirty(dbSymbol);
							}
							m_decCodeViewer->m_codeChanged = true;
						});
					builtinWin = new PopupBuiltinWindow(panel);
					builtinWin->getPos() = m_winPos;
					builtinWin->open();
				}
			}
		};
		
		class CodeGenerator : public CE::Decompiler::CodeViewGenerator
		{
			static void HighlightItem(ColorRGBA color) {
				ImGui::GetWindowDrawList()->AddRectFilled(
					ImGui::GetItemRectMin() - ImVec2(2.0f, 1.0f), ImGui::GetItemRectMax() + ImVec2(2.0f, 1.0f), ToImGuiColorU32(color));
				ImGui::SetCursorScreenPos(ImGui::GetItemRectMin());
			}

			static void FrameItem(ColorRGBA color) {
				ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin() - ImVec2(1.0f, 1.0f), ImGui::GetItemRectMax() + ImVec2(1.0f, 1.0f), ToImGuiColorU32(color));
			}
		public:
			inline const static ColorRGBA COLOR_DEBUG_INFO = 0xd982c2FF;
			inline const static ColorRGBA COLOR_BG_TEXT_ON_HOVER = 0x212e40FF;
			inline const static ColorRGBA COLOR_BG_USER_SELECTED_TEXT = 0x3d0000FF;
			inline const static ColorRGBA COLOR_BG_SELECTED_TEXT = 0x374559FF;
			inline const static ColorRGBA COLOR_FRAME_SELECTED_TEXT = 0x8c8c8cFF;
			inline const static ColorRGBA COLOR_DATA_TYPE = 0xd9c691FF;
			inline const static ColorRGBA COLOR_NUMBER = 0xe6e6e6FF;
		
			class ExprTreeGenerator : public CE::Decompiler::ExprTree::ExprTreeViewGenerator
			{
				CodeGenerator* m_parentGen;
				DecompiledCodeViewer* m_decCodeViewer;
			public:
				MultiLineGroup m_groups;
				
				ExprTreeGenerator(CodeGenerator* parentGen)
					: m_parentGen(parentGen), m_decCodeViewer(parentGen->m_decCodeViewer)
				{}

				void generateToken(const std::string& text, TokenType tokenType) override {
					if (text == " ") {
						if (m_parentGen->checkToGoToNewLine())
							return;
					}
					
					ColorRGBA color = 0xe6e6e6FF;
					if (tokenType == TOKEN_DATA_TYPE)
						color = COLOR_DATA_TYPE;
					else if (tokenType == TOKEN_DEC_SYMBOL)
						color = 0xb7c2b2FF;
					else if (tokenType == TOKEN_FUNCTION_CALL)
						color = 0xdedac5FF;
					else if (tokenType == TOKEN_DEBUG_INFO)
						color = COLOR_DEBUG_INFO;

					m_parentGen->renderToken(text, color);
				}

				void generateSdaSymbol(CE::Symbol::ISymbol* symbol) override {
					renderSdaSymbol(symbol);
					const auto events = GenericEvents(true);

					if (events.isClickedByRightMouseBtn()) {
						auto& ctxWin = m_decCodeViewer->m_ctxWindow;
						delete ctxWin;
						ctxWin = new PopupContextWindow(new SymbolContextPanel(symbol, m_decCodeViewer, GetLeftBottom()));
						ctxWin->open();
						m_parentGen->m_hasNodeSelected = true;
					}

					if (events.isClickedByLeftMouseBtn()) {
						m_decCodeViewer->m_selectedSymbol = symbol;
						m_decCodeViewer->m_selectedSymbolCount = -2;
					}

					const auto hasSymbolSelected = m_decCodeViewer->m_selectedSymbol == symbol;
					if (events.isHovered() || hasSymbolSelected) {
						HighlightItem(events.isHovered() ? COLOR_BG_TEXT_ON_HOVER : COLOR_BG_SELECTED_TEXT);
						m_parentGen->m_isTextAddingOnCurLine++;
						renderSdaSymbol(symbol);
						m_parentGen->m_isTextAddingOnCurLine--;
						if (hasSymbolSelected && m_decCodeViewer->m_selectedSymbolCount != -2)
							m_decCodeViewer->m_selectedSymbolCount ++;
					}

					if (events.isHovered()) {
						ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
						if (const auto funcSymbol = dynamic_cast<CE::Symbol::FunctionSymbol*>(symbol)) {
							if (events.isDoubleClickedByMiddleMouseBtn()) {
								m_decCodeViewer->m_clickedFunction = funcSymbol->getFunction();
							}
						}
						else {
							ImGui::SetTooltip(("Data type: " + symbol->getDataType()->getDisplayName()).c_str());
						}

						if (const auto globalVarSymbol = dynamic_cast<CE::Symbol::GlobalVarSymbol*>(symbol)) {
							if (events.isDoubleClickedByMiddleMouseBtn()) {
								m_decCodeViewer->m_clickedGlobalVar = globalVarSymbol;
							}
						}
					}
				}

				void renderSdaSymbol(CE::Symbol::ISymbol* symbol) const {
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
					m_parentGen->renderToken(symbol->getName(), color);
				}

				void generateNumberLeaf(CE::Decompiler::NumberLeaf* leaf) override {
					ExprTreeViewGenerator::generateNumberLeaf(leaf);
					if (ImGui::IsItemHovered()) {
						HighlightItem(COLOR_BG_TEXT_ON_HOVER);
						m_parentGen->m_isTextAddingOnCurLine ++;
						ExprTreeViewGenerator::generateNumberLeaf(leaf);
						m_parentGen->m_isTextAddingOnCurLine --;
						ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
						RenderNumberPresentations(leaf->getValue());
					}
				}
				
				void generateSdaNumberLeaf(CE::Decompiler::SdaNumberLeaf* leaf) override {
					using namespace Helper::String;
					ExprTreeViewGenerator::generateSdaNumberLeaf(leaf);
					if (ImGui::IsItemHovered()) {
						HighlightItem(COLOR_BG_TEXT_ON_HOVER);
						m_parentGen->m_isTextAddingOnCurLine ++;
						ExprTreeViewGenerator::generateSdaNumberLeaf(leaf);
						m_parentGen->m_isTextAddingOnCurLine --;
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
						m_groups.beginGroup(obj);
					}
					
					ExprTreeViewGenerator::generateRoundBracket(text, obj);
					if(obj == m_decCodeViewer->m_selectedObj) {
						FrameItem(COLOR_FRAME_SELECTED_TEXT);
					}

					if (text == ")") {
						const auto group = m_groups.endGroup();
						if (!m_parentGen->m_hasNodeSelected) {
							if (group.m_events.isClickedByLeftMouseBtn()) {
								m_decCodeViewer->m_selectedObj = obj;
								m_parentGen->m_hasNodeSelected = true;
							}
						}
					}
				}

				void generateNode(CE::Decompiler::INode* node) override {
					bool hasGroup = false;
					MultiLineGroup::Group group;
					if(const auto instrNode = dynamic_cast<CE::Decompiler::IRelatedToInstruction*>(node)) {
						if(!instrNode->getInstructionsRelatedTo().empty()) {
							if (m_decCodeViewer->m_isCodeSelecting) {
								// select code and instructions at the same time
								if (m_parentGen->isSelected()) {
									for (const auto instr : instrNode->getInstructionsRelatedTo())
										m_decCodeViewer->m_selectedInstrs.push_back(instr);
								}
							} else {
								auto isSelected = false;
								// select nodes by the middle mouse button
								if(m_decCodeViewer->m_hoveredNodeOnPrevFrame) {
									isSelected = node->getHash().getHashValue() == m_decCodeViewer->m_hoveredNodeOnPrevFrame->getHash().getHashValue();
								}
								// select code by instruction selection in image viewer
								if(!m_decCodeViewer->m_selectedCodeByInstr.empty()) {
									const auto& instrs = m_decCodeViewer->m_selectedCodeByInstr;
									for(const auto instr : instrNode->getInstructionsRelatedTo()) {
										if(instrs.find(instr) != instrs.end()) {
											isSelected = true;
											break;
										}
									}
								}

								// generate node
								if (isSelected)
									m_parentGen->beginSelecting();
								m_groups.beginGroup(node);
								ExprTreeViewGenerator::generateNode(node);
								group = m_groups.endGroup();
								hasGroup = true;
								if (isSelected)
									m_parentGen->endSelecting();

								// select nodes by the middle mouse button
								if (!m_decCodeViewer->m_hoveredNode) {
									if (group.m_events.isHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f)) {
										m_decCodeViewer->m_hoveredNode = node;
										m_decCodeViewer->m_selectedInstrs = instrNode->getInstructionsRelatedTo();
									}
								}
							}
						}
					}

					if (m_decCodeViewer->m_debugger) {
						if (const auto storagePathNode = dynamic_cast<CE::Decompiler::IStoragePathNode*>(node)) {
							if(!hasGroup) {
								m_groups.beginGroup(node);
								ExprTreeViewGenerator::generateNode(node);
								group = m_groups.endGroup();
								hasGroup = true;
							}
							
							if (group.m_events.isHovered() && !m_decCodeViewer->m_debugger->m_valueViewerWin) {
								ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
								if (m_decCodeViewer->m_hoverTimer) {
									if (GetTimeInMs() - m_decCodeViewer->m_hoverTimer > 200) {
										const auto path = storagePathNode->getStoragePath();
										std::string name = "value";
										CE::DataTypePtr dataType;
										if (const auto sdaNode = dynamic_cast<CE::Decompiler::ISdaNode*>(node)) {
											if (const auto sdaSymbolLeaf = dynamic_cast<CE::Decompiler::SdaSymbolLeaf*>(node))
												name = sdaSymbolLeaf->getSdaSymbol()->getName();
											dataType = sdaNode->getDataType();
										}
										else {
											const auto project = m_decCodeViewer->m_debugger->m_imageDec->getImageManager()->getProject();
											dataType = project->getTypeManager()->getDefaultType(storagePathNode->getSize());
										}

										m_decCodeViewer->m_debugger->createValueViewer(path, name, dataType);
									}
								}
								else {
									m_decCodeViewer->m_hoverTimer = GetTimeInMs();
								}

								m_decCodeViewer->m_isObjHovered = true;
							}
						}
					}

					if (!hasGroup) {
						ExprTreeViewGenerator::generateNode(node);
					}
				}
			};

		private:
			DecompiledCodeViewer* m_decCodeViewer;
			bool m_hasNodeSelected = false;
			std::string m_textOnCurLine;
			int m_curLineIdx = 0;
			int m_curTokenIdx = 0;
			std::list<ColorRGBA> m_selectedCodeColors;
		public:
			ExprTreeGenerator m_exprTreeGenerator;
			int m_isTextAddingOnCurLine = 0;
			
			CodeGenerator(DecompiledCodeViewer* decompiledCodeViewer)
				: CodeViewGenerator(&m_exprTreeGenerator), m_decCodeViewer(decompiledCodeViewer), m_exprTreeGenerator(this)
			{
				setMinInfoToShow();
				m_SHOW_LINEAR_LEVEL_EXT = true;
			}

			void renderToken(const std::string& text, ColorRGBA color) {
				// render token
				Text::ColoredText(text, color).show();

				// selection highlighting
				if (isSelected()) {
					HighlightItem(COLOR_BG_USER_SELECTED_TEXT);
					Text::ColoredText(text, color).show();
					m_decCodeViewer->m_selectedText += text;
				} else {
					if (!m_selectedCodeColors.empty()) {
						HighlightItem(*m_selectedCodeColors.rbegin());
						Text::ColoredText(text, color).show();
					}
				}

				// selection event
				const auto events = GenericEvents(true);
				if (events.isMouseDragging()) {
					if (m_decCodeViewer->m_isCodeSelecting) {
						m_decCodeViewer->m_endSelectionTokenIdx = m_curTokenIdx;
					}
					else {
						m_decCodeViewer->m_startSelectionTokenIdx = m_curTokenIdx;
						m_decCodeViewer->m_isCodeSelecting = true;
					}
				}
				SameLine(1.0f);

				// other
				if (!m_isTextAddingOnCurLine) {
					m_textOnCurLine += text;
					m_curTokenIdx++;
				}
			}


			// is the next generated item in the user selection
			bool isSelected() const {
				if (m_decCodeViewer->m_startSelectionTokenIdx == -1 ||
					m_decCodeViewer->m_endSelectionTokenIdx == -1)
					return false;
				return m_curTokenIdx >= m_decCodeViewer->m_startSelectionTokenIdx &&
					m_curTokenIdx <= m_decCodeViewer->m_endSelectionTokenIdx ||
					m_curTokenIdx <= m_decCodeViewer->m_startSelectionTokenIdx &&
					m_curTokenIdx >= m_decCodeViewer->m_endSelectionTokenIdx;
			}

			void beginSelecting(ColorRGBA color = COLOR_BG_USER_SELECTED_TEXT) {
				m_selectedCodeColors.push_back(color);
			}

			void endSelecting() {
				m_selectedCodeColors.pop_back();
			}

			// if the current line is longer than the window then go to a new line
			bool checkToGoToNewLine() {
				const auto curLineSize = ImGui::CalcTextSize(m_textOnCurLine.c_str()).x;
				if (curLineSize >= std::max(m_decCodeViewer->m_maxLineSize, 400.0f)) {
					m_exprTreeGenerator.m_groups.beginSeparator();
					generateEndLine();
					generateTabs();
					generateTab();
					generateTab();
					m_exprTreeGenerator.m_groups.endSeparator();
					return true;
				}
				return false;
			}

			void generateToken(const std::string& text, TokenType tokenType) override {
				if (tokenType == TOKEN_END_LINE) {
					generateNewLine();
					return;
				}

				if(text == " ") {
					if (checkToGoToNewLine())
						return;
				}
				
				if (text == "\t") {
					generateToken("  ", TOKEN_OTHER);
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
				renderToken(text, color);
				m_textOnCurLine += text;
				m_curTokenIdx++;
			}

			void generateNewLine() {
				m_curLineIdx++;
				m_textOnCurLine = "";

				// line selection
				auto rowColor = ToImGuiColorU32(0x141414FF);
				if(m_decCodeViewer->m_debugSelectedLineIdx == m_curLineIdx - 1) {
					rowColor = ToImGuiColorU32(0x420001FF);
				}
				else if (m_decCodeViewer->m_selectedLineIdx == m_curLineIdx - 1) {
					rowColor = ToImGuiColorU32(0x1d333dFF);
				}
				ImGui::PushStyleColor(ImGuiCol_TableRowBg, rowColor);
				ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, rowColor);
				ImGui::TableNextRow(0, 10.0f);
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();

				// line index
				ImGui::TableNextColumn();
				{
					// process event for line
					if (ImGui::IsWindowHovered()) {
						ImGuiContext& g = *GImGui;
						if (ImGui::IsMousePosValid(&g.IO.MousePos)) {
							const auto table = ImGui::GetCurrentTable();
							const auto startRowPos = ImGui::GetCursorScreenPos();
							const auto rowSize = ImVec2(table->InnerRect.GetSize().x, 17.0f);
							if (ImRect(startRowPos, startRowPos + rowSize).Contains(g.IO.MousePos)) {
								// select line
								if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
									m_decCodeViewer->m_selectedLineIdx = m_curLineIdx;
								}
							}
						}
					}
				}
				Text::ColoredText(std::to_string(m_curLineIdx), 0x7d7d7dFF).show();

				// code
				ImGui::TableNextColumn();
			}

			void generateBlockList(CE::Decompiler::LinearView::BlockList* blockList, bool generatingBraces) override {
				if (m_decCodeViewer->m_hidedBlockLists.find(blockList) != m_decCodeViewer->m_hidedBlockLists.end()) {
					// if the block list is hided by user
					ImGui::BeginGroup();
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
					generateEndLine();
					generateTabs();
					return;
				}
				
				CodeViewGenerator::generateBlockList(blockList, generatingBraces);
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
					generateEndLine();
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
								m_isTextAddingOnCurLine ++;
								m_exprTreeGenerator.generate(regInfo.m_expr->getNode());
								m_isTextAddingOnCurLine --;
							}
						}
						ImGui::EndTable();
					}
					generateEndLine();
				}
			}

			void generateBlockTopNode(CE::Decompiler::DecBlock::BlockTopNode* blockTopNode, CE::Decompiler::INode* node) override {
				const auto debugger = m_decCodeViewer->m_debugger;
				if (debugger && !debugger->m_isStopped) {
					if (debugger->m_curBlockTopNode == blockTopNode) {
						m_decCodeViewer->m_debugSelectedLineIdx = m_curLineIdx;
					}
				}
				CodeViewGenerator::generateBlockTopNode(blockTopNode, node);
			}

			void generateCurlyBracket(const std::string& text, CE::Decompiler::LinearView::BlockList* blockList) override {
				if (text == "}") {
					m_tabsCount--;
					generateTabs();
				}
				
				generateToken(text, TOKEN_CURLY_BRACKET);
				if (blockList == m_decCodeViewer->m_selectedBlockList) {
					FrameItem(COLOR_FRAME_SELECTED_TEXT);
				}
				curlyBracketEvent(blockList);

				if (text == "{") {
					generateEndLine();
					m_tabsCount++;
				}
			}

			void curlyBracketEvent(CE::Decompiler::LinearView::BlockList* blockList) const {
				const auto events = GenericEvents(true);
				if (events.isClickedByLeftMouseBtn()) {
					m_decCodeViewer->m_selectedBlockList = blockList;
				}
				if (events.isClickedByRightMouseBtn() && !m_hasNodeSelected) {
					auto& ctxWin = m_decCodeViewer->m_ctxWindow;
					delete ctxWin;
					ctxWin = new PopupContextWindow(new BlockListContextPanel(blockList, m_decCodeViewer));
					ctxWin->open();
				}
			}
		};

		// for round brackets
		void* m_selectedObj = nullptr;

		// block list
		CE::Decompiler::LinearView::BlockList* m_selectedBlockList = nullptr;
		std::set<CE::Decompiler::LinearView::BlockList*> m_hidedBlockLists;
		
		// symbol
		CE::Symbol::ISymbol* m_selectedSymbol = nullptr;
		int m_selectedSymbolCount = -1;

		// node
		CE::Decompiler::INode* m_selectedNode = nullptr;
		CE::Decompiler::INode* m_hoveredNode = nullptr;
		CE::Decompiler::INode* m_hoveredNodeOnPrevFrame = nullptr;

		// line selection
		int m_debugSelectedLineIdx = -1;
		int m_selectedLineIdx = -1;

		// code selection
		bool m_isCodeSelecting = false;
		int m_startSelectionTokenIdx = -1;
		int m_endSelectionTokenIdx = -1;

		// hover
		bool m_isObjHovered = 0;
		uint64_t m_hoverTimer = 0;

		CE::Decompiler::LinearView::BlockList* m_blockList;
		CE::AbstractImage* m_image = nullptr;
		AbstractInstructionViewDecoder* m_instructionViewDecoder = nullptr;
		CE::Decompiler::PrimaryDecompiler* m_primaryDecompiler = nullptr;

		// windows
		PopupContextWindow* m_ctxWindow = nullptr;
		PopupBuiltinWindow* m_builtinWindow = nullptr;
	public:
		struct
		{
			bool Asm = false;
			bool PCode = false;
			bool ExecCtxs = false;
			bool DebugComments = true;
		} m_show;
		CE::Function* m_clickedFunction = nullptr;
		CE::Symbol::GlobalVarSymbol* m_clickedGlobalVar = nullptr;
		std::list<CE::Decompiler::Instruction*> m_selectedInstrs;
		std::set<CE::Decompiler::Instruction*> m_selectedCodeByInstr;
		std::string m_selectedText;
		bool m_codeChanged = false;
		float m_maxLineSize = 0.0f;
		Debugger* m_debugger = nullptr;
		
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

		bool selectBlockList(CE::Decompiler::DecBlock* decBlock) {
			if(const auto block = m_blockList->findBlock(decBlock)) {
				m_selectedBlockList = block->m_blockList;
				return true;
			}
			return false;
		}

	protected:
		virtual void renderCode() {
			CodeGenerator generator(this);
			generator.m_SHOW_ALL_COMMENTS = m_show.DebugComments;
			generator.generateEndLine();
			generator.generate(m_blockList);
		}

	private:
		void renderControl() override {
			m_selectedInstrs.clear();
			m_selectedText.clear();
			m_hoveredNodeOnPrevFrame = m_hoveredNode;
			m_hoveredNode = nullptr;
			m_clickedFunction = nullptr;
			m_clickedGlobalVar = nullptr;
			m_codeChanged = false;
			m_debugSelectedLineIdx = -1;
			if (!m_isObjHovered)
				m_hoverTimer = 0x0;
			m_isObjHovered = false;
			Show(m_ctxWindow);
			Show(m_builtinWindow);
			
			ImGui::BeginChild(getId().c_str(), ImVec2(0, 0), false, ImGuiWindowFlags_NoMove);
			if (ImGui::IsWindowHovered()) {
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					m_selectedSymbol = nullptr;
					m_selectedSymbolCount = -1;
					m_selectedObj = nullptr;
					m_selectedBlockList = nullptr;
					m_selectedNode = nullptr;
					m_startSelectionTokenIdx = -1;
					m_endSelectionTokenIdx = -1;
					m_selectedCodeByInstr.clear();
				}

				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
					m_isCodeSelecting = false;
				}
			}

			if (ImGui::BeginTable("dec_code_table", 2, ImGuiTableFlags_RowBg))
			{
				ImGui::TableSetupColumn("1", ImGuiTableColumnFlags_WidthFixed, 20.0f);
				ImGui::TableSetupColumn("2", ImGuiTableColumnFlags_None);
				renderCode();
				ImGui::EndTable();
			}
			ImGui::EndChild();

			if(m_selectedSymbolCount != -1) {
				if (m_selectedSymbolCount == -2) {
					m_selectedSymbolCount = 0;
				}
				else if(m_selectedSymbolCount == 1) {
					m_selectedSymbol = nullptr;
					m_selectedSymbolCount = -1;
				}
			}
		}
	};
	
	class DecompiledCodeViewerPanel : public AbstractPanel
	{
		class DecompiledCodeViewerWithHeader : public DecompiledCodeViewer
		{
			CE::Decompiler::SdaCodeGraph* m_sdaCodeGraph;
			CE::Function* m_function;
			std::list<CE::Symbol::ISymbol*> m_symbols;
		public:
			DecompiledCodeViewerWithHeader(CE::Decompiler::LinearView::BlockList* blockList, CE::Decompiler::SdaCodeGraph* sdaCodeGraph, CE::Function* function)
				: DecompiledCodeViewer(blockList), m_sdaCodeGraph(sdaCodeGraph), m_function(function)
			{
				gatherSdaSymbols();
			}

		private:
			void renderCode() override {
				CodeGenerator generator(this);
				generator.m_SHOW_ALL_COMMENTS = m_show.DebugComments;

				// function signature
				generator.generateEndLine();
				renderFunctionSignature(generator.m_exprTreeGenerator);
				generator.generateEndLine();
				generator.generateCurlyBracket("{", m_blockList);

				// header (local variables)
				generator.generateHeader(m_symbols);
				generator.generateEndLine();

				// code
				generator.generate(m_blockList);

				// end
				generator.generateCurlyBracket("}", m_blockList);
				generator.generateEndLine();
			}

			void renderFunctionSignature(CodeGenerator::ExprTreeGenerator& gen) const {
				const auto funcSig = m_function->getSignature();
				gen.generateDataType(funcSig->getReturnType());
				gen.generateToken(" ", CodeGenerator::ExprTreeGenerator::TOKEN_OTHER);
				gen.generateSdaSymbol(m_function->getFunctionSymbol());
				gen.generateRoundBracket("(", funcSig);
				const auto params = funcSig->getParameters();
				for (int i = 0; i < funcSig->getParameters().getParamsCount(); i ++) {
					const auto paramSymbol = funcSig->getParameters()[i];
					gen.generateDataType(paramSymbol->getDataType());
					gen.generateToken(" ", CodeGenerator::ExprTreeGenerator::TOKEN_OTHER);
					gen.generateSdaSymbol(paramSymbol);
					if(i != funcSig->getParameters().getParamsCount() - 1)
						gen.generateToken(", ", CodeGenerator::ExprTreeGenerator::TOKEN_OTHER);
				}
				gen.generateRoundBracket(")", funcSig);
			}

			void gatherSdaSymbols() {
				std::set<CE::Symbol::ISymbol*> symbols;
				m_sdaCodeGraph->gatherAllSdaSymbols(symbols);
				m_symbols.insert(m_symbols.begin(), symbols.begin(), symbols.end());
				m_symbols.remove_if([](CE::Symbol::ISymbol* symbol)
					{
						// remove function result vars with a void type
						if (dynamic_cast<CE::DataType::Void*>(symbol->getDataType()->getType()))
							return true;
						// remove global symbols and parameters
						if (!dynamic_cast<CE::Symbol::LocalStackVarSymbol*>(symbol) && !dynamic_cast<CE::Symbol::LocalInstrVarSymbol*>(symbol))
							return true;
						return false;
					});
				m_symbols.sort([](CE::Symbol::ISymbol* a, CE::Symbol::ISymbol* b) {
					return a->getName() < b->getName();
					});
			}
		};
		
		CE::Decompiler::LinearView::BlockList* m_blockList;
	public:
		DecompiledCodeViewer* m_decompiledCodeViewer;
		bool m_startDebug = false;
		
		DecompiledCodeViewerPanel(CE::Decompiler::LinearView::BlockList* blockList)
			: AbstractPanel("Decompiled Code Viewer"), m_blockList(blockList)
		{}
		
		DecompiledCodeViewerPanel(CE::Decompiler::SdaCodeGraph* sdaCodeGraph, CE::Function* function)
			: DecompiledCodeViewerPanel(CE::Decompiler::Misc::BuildBlockList(sdaCodeGraph->getDecGraph()))
		{
			m_decompiledCodeViewer = new DecompiledCodeViewerWithHeader(m_blockList, sdaCodeGraph, function);
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
			return new StdWindow(this, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_HorizontalScrollbar);
		}

	private:
		void renderPanel() override {
			m_decompiledCodeViewer->m_maxLineSize = m_window->getSize().x - 200.0f;
			m_decompiledCodeViewer->show();
		}

		void renderMenuBar() override {
			m_startDebug = false;
			
			if (ImGui::BeginMenu("Debug"))
			{
				if (ImGui::MenuItem("Start Debug")) {
					m_startDebug = true;
				}
				if (m_decompiledCodeViewer->m_debugger)
					m_decompiledCodeViewer->m_debugger->renderDebugMenu();
				ImGui::EndMenu();
			}
			
			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Show All Blocks")) {
					m_decompiledCodeViewer->m_hidedBlockLists.clear();
				}

				ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
				if (ImGui::MenuItem("Show Assembler", nullptr, m_decompiledCodeViewer->m_show.Asm)) {
					m_decompiledCodeViewer->m_show.Asm ^= true;
				}
				if (m_decompiledCodeViewer->m_show.Asm) {
					if (ImGui::MenuItem("Show PCode", nullptr, m_decompiledCodeViewer->m_show.PCode)) {
						m_decompiledCodeViewer->m_show.PCode ^= true;
					}
				}
				if (ImGui::MenuItem("Show Exec. Contexts", nullptr, m_decompiledCodeViewer->m_show.ExecCtxs)) {
					m_decompiledCodeViewer->m_show.ExecCtxs ^= true;
				}
				if (ImGui::MenuItem("Show Comments", nullptr, m_decompiledCodeViewer->m_show.DebugComments)) {
					m_decompiledCodeViewer->m_show.DebugComments ^= true;
				}
				ImGui::PopItemFlag();
				ImGui::EndMenu();
			}
		}
	};
};