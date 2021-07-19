#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
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
	protected:
		class CodeGenerator : public CE::Decompiler::CodeViewGenerator
		{
			inline const static ColorRGBA COLOR_DEBUG_INFO = 0xd982c2FF;
			inline const static ColorRGBA COLOR_BG_TEXT_ON_HOVER = 0x212e40FF;
			inline const static ColorRGBA COLOR_BG_SELECTED_TEXT = 0x374559FF;
			inline const static ColorRGBA COLOR_FRAME_SELECTED_TEXT = 0x8c8c8cFF;

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
						color = 0xd9c691FF;
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
					auto& selSymbols = m_parentGen->m_decompiledCodeViewer->m_selectedSymbols;
					if (ImGui::IsItemClicked()) {
						selSymbols.insert(symbol);
					}

					if (ImGui::IsItemHovered() || selSymbols.find(symbol) != selSymbols.end()) {
						HighlightItem(ImGui::IsItemHovered() ? COLOR_BG_TEXT_ON_HOVER : COLOR_BG_SELECTED_TEXT);
						ExprTreeViewGenerator::generateSdaSymbol(symbol);
					}

					if (ImGui::IsItemHovered()) {
						ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
						ImGui::SetTooltip(("Data type: " + symbol->getDataType()->getDisplayName()).c_str());
					}
				}

				void generateSdaNumberLeaf(CE::Decompiler::SdaNumberLeaf* leaf) override {
					using namespace Helper::String;
					ExprTreeViewGenerator::generateSdaNumberLeaf(leaf);
					if (ImGui::IsItemHovered()) {
						HighlightItem(COLOR_BG_TEXT_ON_HOVER);
						ExprTreeViewGenerator::generateSdaNumberLeaf(leaf);
						ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
						auto value = leaf->getValue();
						const auto tooltip = std::string("Presentations:\n") +
							"\nhex: " + NumberToHex(value) +
							"\nbinary: " + std::bitset<64>(value).to_string() +
							"\nuint64_t: " + std::to_string(value) +
							"\nint64_t: " + std::to_string(reinterpret_cast<int64_t&>(value)) +
							"\nfloat: " + std::to_string(reinterpret_cast<float&>(value)) +
							"\ndouble: " + std::to_string(reinterpret_cast<double&>(value));
						ImGui::SetTooltip(tooltip.c_str());
					}
				}
			};

			DecompiledCodeViewer* m_decompiledCodeViewer;
			ExprTreeGenerator m_exprTreeGenerator;
			bool m_hasBlockListSelected = false;
		public:
			CodeGenerator(DecompiledCodeViewer* decompiledCodeViewer)
				: CodeViewGenerator(&m_exprTreeGenerator), m_decompiledCodeViewer(decompiledCodeViewer), m_exprTreeGenerator(this)
			{}

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
				if (m_decompiledCodeViewer->m_hidedBlockLists.find(blockList) != m_decompiledCodeViewer->m_hidedBlockLists.end()) {
					ImGui::BeginGroup();
					generateTabs();
					generateToken("{", TOKEN_BRACE);
					generateToken("...", TOKEN_OTHER);
					generateToken("}", TOKEN_BRACE);
					ImGui::EndGroup();
					if (ImGui::IsItemHovered()) {
						ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
						ImGui::SetTooltip("Open");
					}
					if (ImGui::IsItemClicked()) {
						m_decompiledCodeViewer->m_hidedBlockLists.erase(blockList);
					}
					return;
				}
				
				ImGui::BeginGroup();
				CodeViewGenerator::generateBlockList(blockList, generatingTabs, generatingBraces);
				ImGui::EndGroup();
				if (!m_hasBlockListSelected) {
					if (ImGui::IsItemClicked()) {
						m_decompiledCodeViewer->m_selectedBlockList = blockList;
						m_hasBlockListSelected = true;
					}
					if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
						m_decompiledCodeViewer->m_hidedBlockLists.insert(blockList);
						m_hasBlockListSelected = true;
					}
				}
			}

			void generateBrace(const std::string& text, CE::Decompiler::LinearView::BlockList* blockList) override {
				CodeViewGenerator::generateBrace(text, blockList);
				if (blockList == m_decompiledCodeViewer->m_selectedBlockList) {
					FrameItem(COLOR_FRAME_SELECTED_TEXT);
				}
			}
		};

		CE::Decompiler::LinearView::BlockList* m_blockList;
		std::set<CE::Symbol::ISymbol*> m_selectedSymbols; // todo: create a controller?
		CE::Decompiler::LinearView::BlockList* m_selectedBlockList = nullptr;
		std::set<CE::Decompiler::LinearView::BlockList*> m_hidedBlockLists;
	public:
		DecompiledCodeViewer(CE::Decompiler::LinearView::BlockList* blockList)
			: m_blockList(blockList)
		{}

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
					m_selectedBlockList = nullptr;
				}
			}
			
			renderCode();
			ImGui::EndChild();
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
		DecompiledCodeViewerWithHeader* m_decompiledCodeViewer;
	public:
		DecompiledCodeViewerPanel(CE::Decompiler::SdaCodeGraph* sdaCodeGraph)
			: AbstractPanel("Decompiled code viewer")
		{
			m_blockList = CE::Decompiler::Misc::BuildBlockList(sdaCodeGraph->getDecGraph());
			m_decompiledCodeViewer = new DecompiledCodeViewerWithHeader(m_blockList, sdaCodeGraph);
		}

		~DecompiledCodeViewerPanel() override {
			delete m_blockList;
			delete m_decompiledCodeViewer;
		}

		StdWindow* createStdWindow() {
			return new StdWindow(this/*, ImGuiWindowFlags_MenuBar*/);
		}

	private:
		void renderPanel() override {
			m_decompiledCodeViewer->show();
		}
	};
};