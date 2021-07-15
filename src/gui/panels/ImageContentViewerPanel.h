#pragma once
#include "ImageDecorator.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/List.h"
#include "imgui_wrapper/controls/Text.h"
#include "utilities/Helper.h"
#include "InstructionViewer.h"
#include "decompiler/Graph/DecPCodeGraph.h"
#include "panels/FuncGraphViewerPanel.h"

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
	
	class ImageContentViewerPanel : public AbstractPanel
	{
		inline const static ImGuiTableFlags TableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

		class AbstractSectionViewer : public Control
		{
		public:
			AbstractSectionController* m_sectionController;
			
			AbstractSectionViewer(AbstractSectionController* controller)
				: m_sectionController(controller)
			{}
		
		protected:
			void renderAddressColumn(uint64_t offset) {
				ImGui::TableNextColumn();
				RenderAddress(offset);
			}
		};

		class DataSectionViewer : public AbstractSectionViewer
		{
			DataSectionController* m_dataSectionController;
		public:
			DataSectionViewer(DataSectionController* dataSectionController)
				: AbstractSectionViewer(dataSectionController), m_dataSectionController(dataSectionController)
			{}

		private:
			void renderControl() override {
				if (ImGui::BeginTable("content_table", 3, TableFlags))
				{
					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_None);
					ImGui::TableSetupColumn("Data type", ImGuiTableColumnFlags_None);
					ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_None);
					ImGui::TableHeadersRow();

					ImGuiListClipper clipper;
					clipper.Begin(m_dataSectionController->getRowsCount());
					while (clipper.Step())
					{
						for (auto rowIdx = clipper.DisplayStart; rowIdx < clipper.DisplayEnd; rowIdx++) {
							ImGui::TableNextRow();
							const auto offset = m_dataSectionController->getRow(rowIdx);
							renderAddressColumn(offset);
							renderSpecificColumns(offset);
						}
					}
					ImGui::EndTable();
				}
			}

			void renderSpecificColumns(uint64_t offset) const {
				using namespace Helper::String;

				auto symbol = m_dataSectionController->getSymbol(offset);
				const auto pValue = reinterpret_cast<uint64_t*>(&m_dataSectionController->getImageData()[offset]);
				ImGui::TableNextColumn();
				Text::Text(symbol->getDataType()->getDisplayName()).show();
				ImGui::TableNextColumn();
				Text::Text(symbol->getDataType()->getViewValue(*pValue)).show();
			}
		};
		
		class CodeSectionViewer : public AbstractSectionViewer
		{
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
			
			std::set<CodeSectionController::Jmp*> m_shownJmps;
		public:
			CodeSectionController* m_codeSectionController;
			AbstractInstructionViewDecoder* m_instructionViewDecoder;
			CE::Decompiler::FunctionPCodeGraph* m_curFuncPCodeGraph = nullptr;
			
			CodeSectionViewer(CodeSectionController* codeSectionController, AbstractInstructionViewDecoder* instructionViewDecoder)
				: AbstractSectionViewer(codeSectionController), m_codeSectionController(codeSectionController), m_instructionViewDecoder(instructionViewDecoder)
			{}

			~CodeSectionViewer() override {
				delete m_instructionViewDecoder;
			}

		private:
			void renderControl() override {
				m_shownJmps.clear();
				m_curFuncPCodeGraph = nullptr;
				
				auto goToFunc = Button::StdButton("go to func").present();
				if (ImGui::BeginTable("content_table", 3, TableFlags))
				{
					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_None);
					ImGui::TableSetupColumn("Command", ImGuiTableColumnFlags_None);
					ImGui::TableSetupColumn("Operands", ImGuiTableColumnFlags_None);
					ImGui::TableHeadersRow();

					ImGuiListClipper clipper;
					clipper.Begin(m_codeSectionController->getRowsCount());
					while (clipper.Step())
					{
						for (auto rowIdx = clipper.DisplayStart; rowIdx < clipper.DisplayEnd; rowIdx++) {
							ImGui::TableNextRow();
							const auto codeSectionRow = m_codeSectionController->getRow(rowIdx);
							if (rowIdx == (clipper.DisplayStart + clipper.DisplayEnd) / 2) {
								if (const auto pcodeBlock = m_codeSectionController->m_imageDec->getPCodeGraph()->getBlockAtOffset(codeSectionRow.m_fullOffset)) {
									m_curFuncPCodeGraph = pcodeBlock->m_funcPCodeGraph;
								}
							}

							if (!codeSectionRow.m_isPCode) {
								const auto instrOffset = codeSectionRow.m_byteOffset;
								renderAddressColumn(instrOffset);

								InstructionViewInfo instrViewInfo;
								m_instructionViewDecoder->decode(m_codeSectionController->getImageDataByOffset(instrOffset), &instrViewInfo);
								CodeSectionInstructionViewer instructionViewer(&instrViewInfo);
								instructionViewer.renderJmpArrow([&]()
									{
										renderJmpLines(rowIdx, clipper);
									});
								instructionViewer.show();
							} else {
								const auto instrOffset = codeSectionRow.m_fullOffset;
								if (const auto instr = m_codeSectionController->m_imageDec->getInstrPool()->getPCodeInstructionAt(instrOffset)) {
									ImGui::TableNextColumn();
									Text::Text("").show();
									ImGui::TableNextColumn();
									Text::Text("").show();
									ImGui::TableNextColumn();
									Text::Text(instr->printDebug()).show();
								}
							}
						}
					}
					if(goToFunc) {
						ImGui::SetScrollY(m_codeSectionController->getRowIdx(CodeSectionRow(0x21fc000)) * clipper.ItemsHeight);
					}
					ImGui::EndTable();
				}
			}

			void renderJmpLines(const int rowIdx, const ImGuiListClipper& clipper) {
				if (const auto it = m_codeSectionController->m_offsetToJmp.find(m_codeSectionController->getRow(rowIdx).m_fullOffset);
					it != m_codeSectionController->m_offsetToJmp.end()) {
					auto pJmps = it->second;
					for (auto pJmp : pJmps) {
						if (m_shownJmps.find(pJmp) == m_shownJmps.end()) {
							drawJmpLine(rowIdx, pJmp, clipper);
							m_shownJmps.insert(pJmp);
						}
					}
				}
			}

			void drawJmpLine(const int rowIdx, const CodeSectionControllerX86::Jmp* pJmp, const ImGuiListClipper& clipper) const {
				const float JmpLineLeftOffset = 10.0f;
				const float JmpLineGap = 8.0f;
				
				const bool isStart = m_codeSectionController->getRow(rowIdx).m_fullOffset == pJmp->m_startOffset;
				const auto targetRowIdx = m_codeSectionController->getRowIdx(CodeSectionRow(isStart ? pJmp->m_endOffset : pJmp->m_startOffset));

				auto lineColor = ToImGuiColorU32(-1);
				const ImVec2 point1 = { ImGui::GetItemRectMin().x - 2.0f, (ImGui::GetItemRectMin().y + ImGui::GetItemRectMax().y) / 2.0f };
				const ImVec2 point2 = ImVec2(point1.x - pJmp->m_level * JmpLineGap - JmpLineLeftOffset, point1.y);
				const ImVec2 point3 = ImVec2(point2.x, point2.y + clipper.ItemsHeight * (targetRowIdx - rowIdx));
				const ImVec2 point4 = ImVec2(point1.x, point3.y);

				// click event of the jump arrow
				ImGuiContext& g = *GImGui;
				if (ImGui::IsMousePosValid(&g.IO.MousePos)) {
					auto rect = ImRect(point2, { point3.x + JmpLineGap, point3.y });
					if(rect.Min.x > rect.Max.x || rect.Min.y > rect.Max.y)
						rect = ImRect(point3, { point2.x + JmpLineGap, point2.y });
					if (rect.Contains(g.IO.MousePos)) {
						ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
						lineColor = ToImGuiColorU32(0x00bfffFF);
						if(ImGui::IsMouseClicked(0)) {
							const auto offsetToCenter = clipper.ItemsHeight * (clipper.DisplayEnd - clipper.DisplayStart) / 2.0f;
							ImGui::SetScrollY(targetRowIdx * clipper.ItemsHeight - offsetToCenter);
						}
					}
				}


				// render the jump arrow
				ImGuiWindow* window = ImGui::GetCurrentWindow();
				window->DrawList->AddLine(point1, point2, lineColor);
				window->DrawList->AddLine(point2, point3, lineColor);
				window->DrawList->AddLine(point3, point4, lineColor);
				auto arrowPos = isStart ? point4 : point1;
				arrowPos -= ImVec2(7.0f, 3.0f);
				ImGui::RenderArrow(window->DrawList, arrowPos, lineColor, ImGuiDir_Right, 0.7f);
			}
		};
		
		CE::ImageDecorator* m_imageDec;
		AbstractSectionViewer* m_imageSectionViewer = nullptr;
		std::map<const CE::ImageSection*, AbstractSectionController*> m_imageSectionControllers; // todo: move out of the scope
		ImageSectionListModel m_imageSectionListModel;
		MenuListView<const CE::ImageSection*> m_imageSectionMenuListView;
		StdWindow* m_funcGraphViewerWindow = nullptr;
	public:
		ImageContentViewerPanel(CE::ImageDecorator* imageDec)
			: AbstractPanel("Image content viewer"), m_imageDec(imageDec), m_imageSectionListModel(imageDec->getImage())
		{
			m_imageSectionMenuListView = MenuListView(&m_imageSectionListModel);
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
		}

	private:
		void renderPanel() override {
			Show(m_funcGraphViewerWindow);
			m_imageSectionViewer->show();
		}

		void renderMenuBar() override {
			if (ImGui::BeginMenu("This section"))
			{
				if(ImGui::MenuItem("Update")) {
					m_imageSectionViewer->m_sectionController->update();
				}
				if(auto codeSectionViewer = dynamic_cast<CodeSectionViewer*>(m_imageSectionViewer)) {
					if (ImGui::MenuItem("Show PCode", 0, codeSectionViewer->m_codeSectionController->m_showPCode)) {
						codeSectionViewer->m_codeSectionController->m_showPCode ^= true;
						codeSectionViewer->m_codeSectionController->update();
					}
					if (codeSectionViewer->m_curFuncPCodeGraph) {
						if (ImGui::MenuItem("Show function graph", 0)) {
							if (!m_funcGraphViewerWindow) {
								auto funcGraphViewerPanel = new FuncGraphViewerPanel(m_imageDec, codeSectionViewer->m_instructionViewDecoder);
								funcGraphViewerPanel->setFuncGraph(codeSectionViewer->m_curFuncPCodeGraph);
								m_funcGraphViewerWindow = funcGraphViewerPanel->createStdWindow();
							}
							else {
								if (auto funcGraphViewerPanel = dynamic_cast<FuncGraphViewerPanel*>(m_funcGraphViewerWindow->getPanel())) {
									funcGraphViewerPanel->setFuncGraph(codeSectionViewer->m_curFuncPCodeGraph);
								}
							}
						}
					}
				}
				ImGui::EndMenu();
			}
			
			if (ImGui::BeginMenu("All sections"))
			{
				m_imageSectionMenuListView.show();
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Navigation"))
			{
				if (ImGui::MenuItem("Go to")) {
					
				}
				ImGui::EndMenu();
			}
		}

		void selectImageSection(const CE::ImageSection* imageSection) {
			delete m_imageSectionViewer;
			m_imageSectionMenuListView.m_selectedItem = imageSection;

			AbstractSectionController* controller = nullptr;
			if (const auto it = m_imageSectionControllers.find(imageSection);
				it != m_imageSectionControllers.end())
				controller = it->second;

			if (imageSection->m_type == CE::ImageSection::CODE_SEGMENT) {
				auto codeController = dynamic_cast<CodeSectionControllerX86*>(controller);
				if (!codeController)
					m_imageSectionControllers[imageSection] = codeController = new CodeSectionControllerX86(m_imageDec, imageSection);
				m_imageSectionViewer = new CodeSectionViewer(codeController, new InstructionViewDecoderX86);
			}
			else {
				auto dataController = dynamic_cast<DataSectionController*>(controller);
				if (!dataController)
					m_imageSectionControllers[imageSection] = dataController = new DataSectionController(m_imageDec, imageSection);
				m_imageSectionViewer = new DataSectionViewer(dataController);
			}
		}
	};
};