#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "DataTypeManagerPanel.h"
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
			class NamePanel : public AbstractPanel
			{
			protected:
				SymbolContextPanel* m_ctx;
				Input::TextInput m_input;
			public:
				NamePanel(SymbolContextPanel* ctx)
					: m_ctx(ctx)
				{
					m_input.setInputText(m_ctx->m_symbol->getName());
					m_input.focus();
				}

			private:
				void renderPanel() override {
					m_input.show();
					SameLine();
					if(Button::StdButton("Ok").present()) {
						m_ctx->m_symbol->setName(m_input.getInputText());
						m_ctx->m_decCodeViewer->m_codeChanged = true;
						m_window->close();
						
					}
				}
			};

			class DataTypePanel : public NamePanel
			{
				DataTypeManagerController m_controller;
				TableListViewSelector<CE::DataType::IType*>* m_tableListViewSelector;
				CE::TypeManager* m_typeManager;
			public:
				DataTypePanel(SymbolContextPanel* ctx)
					: NamePanel(ctx), m_controller(ctx->m_symbol->getDataType()->getTypeManager()), m_typeManager(ctx->m_symbol->getDataType()->getTypeManager())
				{
					m_input.setInputText(m_ctx->m_symbol->getDataType()->getName());
					m_controller.m_maxItemsCount = 10;
					
					auto tableListView = new TableListView(&m_controller.m_listModel, {
						ColInfo("Data type", ImGuiTableColumnFlags_WidthFixed, 150.0f),
						ColInfo("Group", ImGuiTableColumnFlags_WidthFixed, 70.0f)
						});
					m_tableListViewSelector = new TableListViewSelector(tableListView);
					m_tableListViewSelector->handler([&](CE::DataType::IType* type)
						{
							m_input.setInputText(type->getName());
						});
				}

				~DataTypePanel() {
					delete m_tableListViewSelector;
				}

			private:
				void renderPanel() override {
					m_input.show();
					SameLine();
					if (Button::StdButton("Ok").present()) {
						// todo: check new size that can overlap other symbols
						const auto dataType = parseDataType(m_input.getInputText());
						m_ctx->m_symbol->setDataType(dataType);
						m_ctx->m_decCodeViewer->m_codeChanged = true;
						m_window->close();
					}
					if(m_controller.hasItems())
						m_tableListViewSelector->show();

					if (m_input.isTextEntering()) {
						m_controller.m_filter.m_name = m_input.getInputText();
						m_controller.update();
					}
				}

				CE::DataTypePtr parseDataType(const std::string& text) const {
					std::string typeName;
					std::string typePtrLevels;
					bool isTypeName = true;
					for (auto c : text) {
						if (c == '*' || c == '[')
							isTypeName = false;
						if (isTypeName) {
							typeName.push_back(c);
						}
						else {
							typePtrLevels.push_back(c);
						}
					}
					if(const auto dataType = m_typeManager->findTypeByName(typeName))
						return GetUnit(dataType, typePtrLevels);
					return nullptr;
				}
			};
			
			CE::Symbol::ISymbol* m_symbol;
		public:
			SymbolContextPanel(CE::Symbol::ISymbol* symbol, DecompiledCodeViewer* decCodeViewer, ImVec2 winPos)
				: AbstractContextPanel(decCodeViewer, winPos), m_symbol(symbol)
			{}

		private:
			void renderPanel() override {
				auto& builtinWin = m_decCodeViewer->m_builtinWindow;
				if (ImGui::MenuItem("Change name")) {
					delete builtinWin;
					builtinWin = new PopupBuiltinWindow(new NamePanel(this));
					builtinWin->getPos() = m_winPos;
					builtinWin->open();
				}
				if (ImGui::MenuItem("Change data type")) {
					delete builtinWin;
					builtinWin = new PopupBuiltinWindow(new DataTypePanel(this));
					builtinWin->getPos() = m_winPos;
					builtinWin->open();
				}
			}
		};
		
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

		public:
			class ExprTreeGenerator : public CE::Decompiler::ExprTree::ExprTreeViewGenerator
			{
				CodeGenerator* m_parentGen;
			public:
				ExprTreeGenerator(CodeGenerator* parentGen)
					: m_parentGen(parentGen)
				{}

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
					const auto events = GenericEvents(true);
					
					if (events.isClickedByRightMouseBtn()) {
						auto& ctxWin = m_parentGen->m_decCodeViewer->m_ctxWindow;
						delete ctxWin;
						ctxWin = new PopupContextWindow(new SymbolContextPanel(symbol, m_parentGen->m_decCodeViewer, GetLeftBottom()));
						ctxWin->open();
						m_parentGen->m_hasNodeSelected = true;
					}
					
					auto& selSymbols = m_parentGen->m_decCodeViewer->m_selectedSymbols;
					if (events.isClickedByLeftMouseBtn()) {
						selSymbols[symbol] = 1000;
					}

					const auto hasSymbolSelected = selSymbols.find(symbol) != selSymbols.end();
					if (events.isHovered() || hasSymbolSelected) {
						HighlightItem(events.isHovered() ? COLOR_BG_TEXT_ON_HOVER : COLOR_BG_SELECTED_TEXT);
						RenderSdaSymbol(symbol);
						if (hasSymbolSelected)
							selSymbols[symbol]++;
					}

					if (events.isHovered()) {
						if (!dynamic_cast<CE::Symbol::FunctionSymbol*>(symbol)) {
							ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
							ImGui::SetTooltip(("Data type: " + symbol->getDataType()->getDisplayName()).c_str());
						}
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

		private:
			DecompiledCodeViewer* m_decCodeViewer;
			bool m_hasNodeSelected = false;
			bool m_hasBlockListSelected = false;
		public:
			ExprTreeGenerator m_exprTreeGenerator;
			
			CodeGenerator(DecompiledCodeViewer* decompiledCodeViewer)
				: CodeViewGenerator(&m_exprTreeGenerator), m_decCodeViewer(decompiledCodeViewer), m_exprTreeGenerator(this)
			{
				setMinInfoToShow();
				m_SHOW_LINEAR_LEVEL_EXT = true;
			}

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

			void generateBlockList(CE::Decompiler::LinearView::BlockList* blockList, bool generatingBraces) override {
				if (m_decCodeViewer->m_hidedBlockLists.find(blockList) != m_decCodeViewer->m_hidedBlockLists.end()) {
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
					m_tabs.push_back('\t');
				}
				
				if (text == "}") {
					m_tabs.pop_back();
					generateTabs();
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
					const auto events = GenericEvents(true);
					if (events.isClickedByLeftMouseBtn()) {
						m_decCodeViewer->m_selectedBlockList = blockList;
						m_hasBlockListSelected = true;
					}
					if (events.isClickedByRightMouseBtn() && !m_hasNodeSelected) {
						auto& ctxWin = m_decCodeViewer->m_ctxWindow;
						delete ctxWin;
						ctxWin = new PopupContextWindow(new BlockListContextPanel(blockList, m_decCodeViewer));
						ctxWin->open();
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
		bool m_codeChanged = false;
		
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
			m_codeChanged = false;
			Show(m_ctxWindow);
			Show(m_builtinWindow);
			
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
				renderFunctionSignature(generator.m_exprTreeGenerator);
				generator.generateEndLine();
				generator.generateCurlyBracket("{", m_blockList);
				generator.generateHeader(m_symbols);
				generator.generateEndLine();
				generator.generate(m_blockList);
				generator.generateCurlyBracket("}", m_blockList);
			}

			void renderFunctionSignature(CodeGenerator::ExprTreeGenerator& gen) const {
				const auto funcSig = m_function->getSignature();
				gen.generateDataType(funcSig->getReturnType());
				gen.generateToken(" ", CodeGenerator::ExprTreeGenerator::TOKEN_OTHER);
				gen.generateSdaSymbol(m_function->getFunctionSymbol());
				gen.generateRoundBracket("(", funcSig);
				const auto params = funcSig->getParameters();
				for (auto it = params.begin(); it != params.end(); ++it) {
					const auto paramSymbol = *it;
					gen.generateDataType(paramSymbol->getDataType());
					gen.generateToken(" ", CodeGenerator::ExprTreeGenerator::TOKEN_OTHER);
					gen.generateSdaSymbol(paramSymbol);
					if(it != std::prev(params.end()))
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
		
		DecompiledCodeViewerPanel(CE::Decompiler::LinearView::BlockList* blockList)
			: AbstractPanel("Decompiled code viewer"), m_blockList(blockList)
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