#pragma once
#include "DebugerPanel.h"
#include "Exception.h"
#include "ImageDecorator.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/List.h"
#include "imgui_wrapper/controls/Text.h"
#include "utilities/Helper.h"
#include "InstructionViewer.h"

namespace GUI
{
	class ImageSectionListModel : public IListModel<const CE::ImageSection*>
	{
		class Iterator : public IListModel<const CE::ImageSection*>::Iterator
		{
			 std::list<CE::ImageSection>::const_iterator m_it;
		protected:
			ImageSectionListModel* m_listModel;
		public:
			Iterator(ImageSectionListModel* listModel)
				: m_listModel(listModel), m_it(listModel->m_image->getImageSections().begin())
			{}

			void getNextItem(std::string* text, const CE::ImageSection** data) override
			{
				*text = m_it->m_name;
				*data = &(*m_it);
				++m_it;
			}

			bool hasNextItem() override
			{
				return m_it != m_listModel->m_image->getImageSections().end();
			}
		};

		CE::AbstractImage* m_image;
	public:
		ImageSectionListModel(CE::AbstractImage* image)
			: m_image(image)
		{}

		void newIterator(const IteratorCallback& callback) override
		{
			Iterator iterator(this);
			callback(&iterator);
		}
	};

	class ProjectPanel;
	class ImageContentViewerPanel : public AbstractPanel
	{
		inline const static ImGuiTableFlags TableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

		class GoToPanel : public AbstractPanel
		{
			ImageContentViewerPanel* m_imageContentViewerPanel;
			Input::TextInput m_offsetInput;
			std::string m_errorMessage;
		public:
			GoToPanel(ImageContentViewerPanel* imageContentViewerPanel)
				: AbstractPanel("Go to"), m_imageContentViewerPanel(imageContentViewerPanel)
			{}

		private:
			void renderPanel() override {
				if(!m_errorMessage.empty())
					Text::Text("Error: " + m_errorMessage).show();
				m_offsetInput.show();
				SameLine();
				if (Button::StdButton("Go").present()) {
					const auto offsetStr = m_offsetInput.getInputText();
					const auto offset = ParseOffset(offsetStr);
					try {
						m_imageContentViewerPanel->goToOffset(offset);
						m_window->close();
					} catch (WarningException& ex) {
						m_errorMessage = ex.what();
					}
				}
			}

			static CE::Offset ParseOffset(const std::string& offsetStr) {
				using namespace Helper::String;
				return offsetStr[1] == 'x' ? static_cast<int>(HexToNumber(offsetStr)) : std::stoi(offsetStr);
			}
		};
		
		class AbstractSectionViewer : public Control
		{
		protected:
			ProjectPanel* m_projectPanel;
			ImGuiListClipper m_clipper;
			int m_scrollToRowIdx = -1;
			bool m_selectCurRow = false;
			ColorRGBA m_selectedRowColor = 0;
			int m_startSelectionRowIdx = -1;
		public:
			AbstractSectionController* m_sectionController;
			
			AbstractSectionViewer(ProjectPanel* projectPanel, AbstractSectionController* controller)
				: m_projectPanel(projectPanel), m_sectionController(controller)
			{}

			void goToOffset(CE::Offset offset) {
				const auto rowIdx = getRowIdxByOffset(offset);
				if (rowIdx == -1)
					throw WarningException("Offset not found.");
				m_scrollToRowIdx = rowIdx;
			}

			bool isRowsMultiSelecting() const {
				return m_startSelectionRowIdx != -1;
			}
		protected:
			virtual int getRowIdxByOffset(CE::Offset offset) = 0;
			
			void scroll() {
				if (m_scrollToRowIdx != -1) {
					ImGui::SetScrollY(m_scrollToRowIdx * m_clipper.ItemsHeight);
					m_scrollToRowIdx = -1;
				}
			}

			void tableNextRow() {
				const auto Color = ToImGuiColorU32(m_selectedRowColor ? m_selectedRowColor : 0x1d333dFF);
				if (m_selectCurRow) {
					ImGui::PushStyleColor(ImGuiCol_TableRowBg, Color);
					ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, Color);
				}
				ImGui::TableNextRow();
				if (m_selectCurRow) {
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
					m_selectCurRow = false;
					m_selectedRowColor = 0;
				}
			}
		};

		class DataSectionViewer : public AbstractSectionViewer
		{
			DataSectionController* m_dataSectionController;
		public:
			DataSectionViewer(ProjectPanel* projectPanel, DataSectionController* dataSectionController)
				: AbstractSectionViewer(projectPanel, dataSectionController), m_dataSectionController(dataSectionController)
			{}

		private:
			void renderControl() override;

			void renderSpecificColumns(uint64_t offset) const {
				using namespace Helper::String;

				auto symbol = m_dataSectionController->getSymbol(offset);
				std::vector<uint8_t> buffer(0x8);
				m_dataSectionController->m_imageDec->getImage()->read(offset, buffer);
				const auto pValue = reinterpret_cast<uint64_t*>(buffer.data());
				ImGui::TableNextColumn();
				Text::Text(symbol->getDataType()->getDisplayName()).show();
				ImGui::TableNextColumn();
				Text::Text("some value").show();
			}

			int getRowIdxByOffset(CE::Offset offset) override {
				return m_dataSectionController->getRowIdx(offset);
			}
		};
		
		class CodeSectionViewer : public AbstractSectionViewer
		{
			class FunctionReferencesPanel : public AbstractPanel
			{
				class FuncCallListModel : public IListModel<CE::Function*>
				{
					CE::Decompiler::FunctionPCodeGraph* m_funcPCodeGraph;
					CE::ImageDecorator* m_imageDec;
				public:
					FuncCallListModel(CE::Function* function)
						: m_funcPCodeGraph(function->getFuncGraph()), m_imageDec(function->getImage())
					{}

					bool empty() const {
						return m_funcPCodeGraph->getRefFuncCalls().empty();
					}
				private:
					class FuncCallIterator : public Iterator
					{
						std::set<CE::Decompiler::FunctionPCodeGraph*>::iterator m_it;
						const std::set<CE::Decompiler::FunctionPCodeGraph*>* m_list;
						CE::ImageDecorator* m_imageDec;
					public:
						FuncCallIterator(const std::set<CE::Decompiler::FunctionPCodeGraph*>* list, CE::ImageDecorator* imageDec)
							: m_list(list), m_it(list->begin()), m_imageDec(imageDec)
						{}

						void getNextItem(std::string* text, CE::Function** data) override {
							const auto funcGraph = *m_it;
							++m_it;
							if (const auto function = m_imageDec->getFunctionAt(funcGraph->getStartBlock()->getMinOffset().getByteOffset())) {
								*text = function->getName();
								*data = function;
							} else {
								if(hasNextItem())
									getNextItem(text, data);
							}
						}

						bool hasNextItem() override {
							return m_it != m_list->end();
						}
					};

					void newIterator(const IteratorCallback& callback) override {
						FuncCallIterator iterator(&m_funcPCodeGraph->getRefFuncCalls(), m_imageDec);
						callback(&iterator);
					}
				};
				
				CE::Function* m_function;
				CodeSectionViewer* m_codeSectionViewer;
				FuncCallListModel m_listModel;
				SelectableTableListView<CE::Function*>* m_listView;
				std::string m_errorMessage;
			public:
				FunctionReferencesPanel(CE::Function* function, CodeSectionViewer* codeSectionViewer)
					: AbstractPanel("Function References"), m_function(function), m_codeSectionViewer(codeSectionViewer), m_listModel(function)
				{
					m_listView = new SelectableTableListView(&m_listModel, {
						ColInfo("Function")
					});
					m_listView->handler([&](CE::Function* function)
						{
							try {
								m_codeSectionViewer->goToOffset(function->getOffset());
								m_window->close();
							}
							catch (WarningException& ex) {
								m_errorMessage = ex.what();
							}
						});
				}

			private:
				void renderPanel() override {
					if (!m_errorMessage.empty())
						Text::Text("Error: " + m_errorMessage).show();
					
					if (!m_listModel.empty()) {
						m_listView->show();
					} else {
						Text::Text("No functions referenced to.").show();
					}
				}
			};
			
			class CodeSectionInstructionViewer : public InstructionTableRowViewer
			{
				EventHandler<> m_renderJmpArrow;
			public:
				using InstructionTableRowViewer::InstructionTableRowViewer;
				
				void renderJmpArrow(const std::function<void()>& renderJmpArrow)
				{
					m_renderJmpArrow = renderJmpArrow;
				}
			
			protected:
				void renderMnemonic() override {
					InstructionTableRowViewer::renderMnemonic();
					m_renderJmpArrow();
				}
			};

			class RowContextPanel : public AbstractPanel
			{
				CodeSectionViewer* m_codeSectionViewer;
				CodeSectionRow m_codeSectionRow;
			public:
				RowContextPanel(CodeSectionViewer* codeSectionViewer, CodeSectionRow codeSectionRow)
					: m_codeSectionViewer(codeSectionViewer), m_codeSectionRow(codeSectionRow)
				{}

			private:
				void renderPanel() override;
			};
			
			std::set<CodeSectionController::Jmp*> m_shownJmps;
			ImGuiWindow* m_window = nullptr;
			PopupBuiltinWindow* m_builtinWindow = nullptr;
			PopupModalWindow* m_popupModalWindow = nullptr;
			PopupContextWindow* m_ctxWindow = nullptr;
		public:
			CodeSectionController* m_codeSectionController;
			AbstractInstructionViewDecoder* m_instructionViewDecoder;
			CE::Decompiler::FunctionPCodeGraph* m_selectedFuncPCodeGraph = nullptr;
			bool m_obscureUnknownLocation = true;
			std::list<CodeSectionRow> m_selectedRows;
			bool m_pcodeGraphChanged = false;
			CE::Decompiler::PCodeBlock* m_clickedPCodeBlock = nullptr;
			bool m_createEmulator = false;
			
			CodeSectionViewer(ProjectPanel* projectPanel, CodeSectionController* codeSectionController, AbstractInstructionViewDecoder* instructionViewDecoder)
				: AbstractSectionViewer(projectPanel, codeSectionController), m_codeSectionController(codeSectionController), m_instructionViewDecoder(instructionViewDecoder)
			{}

			~CodeSectionViewer() override {
				delete m_instructionViewDecoder;
				delete m_builtinWindow;
				delete m_popupModalWindow;
				delete m_ctxWindow;
			}

		private:
			void renderControl() override;

			void decorateRow(const ImVec2& startRowPos, const ImVec2& rowSize, CodeSectionRow row, int rowIdx,
			                 CE::Decompiler::PCodeBlock* pcodeBlock);

			void renderFunctionHeader(CE::Function* function, const ImVec2& startRowPos, const ImVec2& endRowPos,
			                          bool& isEventProcessed);

			void renderJmpLines(const int rowIdx, const ImGuiListClipper& clipper);

			void drawJmpLine(const int rowIdx, const CodeSectionControllerX86::Jmp* pJmp,
			                 const ImGuiListClipper& clipper) const;

			int getRowIdxByOffset(CE::Offset offset) override {
				return m_codeSectionController->getRowIdx(CodeSectionRow(CE::ComplexOffset(offset, 0)));
			}
		};

		enum class DecompilerStep
		{
			DECOMPILING,
			PROCESSING,
			SYMBOLIZING,
			FINAL_PROCESSING,
			//DEFAULT = FINAL_PROCESSING
			DEFAULT = PROCESSING
		};

		enum class ProcessingStep
		{
			CONDITION_PROCESSING		= 1 << 0,
			EXPR_OPTIMIZING				= 1 << 1,
			PAR_ASSIGNMENT_CREATING		= 1 << 2,
			ORDER_FIXING				= 1 << 3,
			VIEW_OPTIMIZING				= 1 << 4,
			LINE_EXPANDING				= 1 << 5,
			USELESS_LINES_REMOVING		= 1 << 6,
			DEBUG_PROCESSING			= 1 << 7,
			DEFAULT = -1 & ~USELESS_LINES_REMOVING
		};

		enum class SymbolizingStep
		{
			BUILDING,
			CALCULATING,
			DEFAULT = CALCULATING
		};

		enum class FinalProcessingStep
		{
			MEMORY_OPTIMIZING			= 1 << 0,
			USELESS_LINES_OPTIMIZING	= 1 << 1,
			DEFAULT = MEMORY_OPTIMIZING | USELESS_LINES_OPTIMIZING
		};

		ProjectPanel* m_projectPanel;
		AbstractSectionViewer* m_imageSectionViewer = nullptr;
		std::map<const CE::ImageSection*, AbstractSectionController*> m_imageSectionControllers; // todo: move out of the scope
		ImageSectionListModel m_imageSectionListModel;
		MenuListView<const CE::ImageSection*> m_imageSectionMenuListView;
		StdWindow* m_funcGraphViewerWindow = nullptr;
		StdWindow* m_decompiledCodeViewerWindow = nullptr;
		PopupModalWindow* m_popupModalWindow = nullptr;

		DecompilerStep m_decompilerStep = DecompilerStep::DEFAULT;
		ProcessingStep m_processingStep = ProcessingStep::DEFAULT;
		SymbolizingStep m_symbolizingStep = SymbolizingStep::DEFAULT;
		FinalProcessingStep m_finalProcessingStep = FinalProcessingStep::DEFAULT;
	public:
		CE::ImageDecorator* m_imageDec;
		CE::Decompiler::DecompiledCodeGraph* m_curDecGraph = nullptr;
		
		ImageContentViewerPanel(CE::ImageDecorator* imageDec, ProjectPanel* projectPanel)
			: AbstractPanel(std::string("Image: ") + imageDec->getName() + "###ImageContentViewer"), m_imageDec(imageDec), m_projectPanel(projectPanel), m_imageSectionListModel(imageDec->getImage())
		{
			m_imageSectionMenuListView = MenuListView<const CE::ImageSection*>(&m_imageSectionListModel);
			m_imageSectionMenuListView.handler([&](const CE::ImageSection* imageSection)
				{
					selectImageSection(imageSection);
				});

			// select the code segment by default
			for(const auto& section : imageDec->getImage()->getImageSections()) {
				if(section.m_type == CE::ImageSection::CODE_SEGMENT) {
					selectImageSection(&section);
					break;
				}
			}
		}

		~ImageContentViewerPanel() override {
			delete m_imageSectionViewer;
			for (const auto& pair : m_imageSectionControllers)
				delete pair.second;
			delete m_funcGraphViewerWindow;
			delete m_decompiledCodeViewerWindow;
			delete m_popupModalWindow;
		}

		StdWindow* createStdWindow() {
			return new StdWindow(this, ImGuiWindowFlags_MenuBar);
		}

		void goToOffset(CE::Offset offset) {
			const auto section = m_imageDec->getImage()->getSectionByOffset(offset);
			if (section->m_type == CE::ImageSection::NONE_SEGMENT)
				throw WarningException("Offset not found.");
			selectImageSection(section);
			m_imageSectionViewer->goToOffset(offset);
		}

		void decompile(CE::Decompiler::FunctionPCodeGraph* functionPCodeGraph);

	private:
		void renderPanel() override {
			m_imageSectionViewer->show();

			processSectionViewerEvents();
			processDecompiledCodeViewerEvents();

			Show(m_funcGraphViewerWindow);
			Show(m_decompiledCodeViewerWindow);
			Show(m_popupModalWindow);
		}

		void renderMenuBar() override;

		void selectImageSection(const CE::ImageSection* imageSection);

		// e.g. if rows were selected
		void processSectionViewerEvents();

		// e.g. if a symbol was renamed
		void processDecompiledCodeViewerEvents();
	};
};