#include "ImageContentViewerPanel.h"
#include "decompiler/Decompiler.h"
#include "decompiler/Graph/DecPCodeGraph.h"
#include "decompiler/Optimization/Graph/DecGraphDebugProcessing.h"
#include "decompiler/SDA/Optimizaton/SdaGraphMemoryOptimization.h"
#include "decompiler/SDA/Optimizaton/SdaGraphUselessLineOptimization.h"
#include "decompiler/SDA/Symbolization/DecGraphSdaBuilding.h"
#include "decompiler/SDA/Symbolization/SdaGraphDataTypeCalc.h"
#include "panels/FuncGraphViewerPanel.h"
#include "panels/DecompiledCodeViewerPanel.h"
#include "ProjectPanel.h"

void GUI::ImageContentViewerPanel::DataSectionViewer::renderControl() {
	ImGui::BeginChild("##empty", ImVec2(0, 0), false, ImGuiWindowFlags_NoMove);
	if (ImGui::BeginTable("##empty", 3, TableFlags)) {
		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_None);
		ImGui::TableSetupColumn("Data type", ImGuiTableColumnFlags_None);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_None);
		ImGui::TableHeadersRow();

		m_clipper.Begin(m_dataSectionController->getRowsCount());
		while (m_clipper.Step()) {
			for (auto rowIdx = m_clipper.DisplayStart; rowIdx < m_clipper.DisplayEnd; rowIdx++) {
				tableNextRow();
				const auto offset = m_dataSectionController->getRow(rowIdx);
				ImGui::TableNextColumn();
				RenderAddress(offset);
				renderSpecificColumns(offset);
			}
		}

		scroll();
		ImGui::EndTable();
	}
	ImGui::EndChild();
}

GUI::ImageContentViewerPanel::CodeSectionViewer::FunctionReferencesPanel::FunctionReferencesPanel(
	CE::Function* function, CodeSectionViewer* codeSectionViewer): AbstractPanel("Function References"),
	                                                               m_function(function),
	                                                               m_codeSectionViewer(codeSectionViewer),
	                                                               m_listModel(function) {
	m_listView = new SelectableTableListView(&m_listModel, {
		                                         ColInfo("Function")
	                                         });
	m_listView->handler([&](CE::Function* function)
	{
		try {
			m_codeSectionViewer->goToOffset(function->getOffset());
			m_codeSectionViewer->m_projectPanel->addVisitedLocation(function->getImage(), function->getOffset());
			m_window->close();
		}
		catch (WarningException& ex) {
			m_errorMessage = ex.what();
		}
	});
}

void GUI::ImageContentViewerPanel::CodeSectionViewer::RowContextPanel::renderPanel() {
	if (ImGui::MenuItem("Analyze")) {
		delete m_codeSectionViewer->m_popupModalWindow;
		const auto panel = new ImageAnalyzerPanel(m_codeSectionViewer->m_codeSectionController->m_imageDec,
		                                          m_codeSectionRow.m_byteOffset);
		m_codeSectionViewer->m_popupModalWindow = new PopupModalWindow(panel);
		m_codeSectionViewer->m_popupModalWindow->open();
	}

	const auto imageDec = m_codeSectionViewer->m_codeSectionController->m_imageDec;
	if (const auto emulator = m_codeSectionViewer->m_projectPanel->getEmulator()) {
		if (ImGui::MenuItem("Set RIP here")) {
			emulator->goDebug(imageDec, m_codeSectionRow.getOffset());
		}
	}
	else {
		if (m_codeSectionViewer->m_codeSectionController->m_imageDec->getInstrPool()->getPCodeInstructionAt(m_codeSectionRow.getOffset())) {
			if (ImGui::MenuItem("Start Emulator here")) {
				m_codeSectionViewer->m_createEmulator = true;
			}
		}
	}

	if (!m_codeSectionRow.m_isPCode) {
		if (imageDec->hasBreakpoint(m_codeSectionRow.m_byteOffset)) {
			if (ImGui::MenuItem("Remove Breakpoint")) {
				imageDec->setBreakpoint(m_codeSectionRow.m_byteOffset, false);
			}
		}
		else {
			if (ImGui::MenuItem("Add Breakpoint")) {
				imageDec->setBreakpoint(m_codeSectionRow.m_byteOffset, true);
			}
		}
	}
}

void GUI::ImageContentViewerPanel::CodeSectionViewer::renderControl() {
	m_shownJmps.clear();
	m_clickedPCodeBlock = nullptr;
	m_pcodeGraphChanged = false;

	if (Button::StdButton("go to func").present()) {
		goToOffset(0x1240);
	}

	ImGui::BeginChild("##empty", ImVec2(0, 0), false, ImGuiWindowFlags_NoMove);
	if (ImGui::BeginTable("##empty", 3, TableFlags)) {
		m_window = ImGui::GetCurrentWindow();
		const auto tableSize = ImGui::GetItemRectSize();
		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_None);
		ImGui::TableSetupColumn("Command", ImGuiTableColumnFlags_None);
		ImGui::TableSetupColumn("Operands", ImGuiTableColumnFlags_None);
		ImGui::TableHeadersRow();

		m_clipper.Begin(m_codeSectionController->getRowsCount() + 2);
		while (m_clipper.Step()) {
			for (auto rowIdx = m_clipper.DisplayStart - 1; rowIdx < m_clipper.DisplayEnd - 1; rowIdx++) {
				tableNextRow();

				// add extra rows
				if(rowIdx == -1 || rowIdx == m_codeSectionController->getRowsCount()) {
					ImGui::TableNextColumn();
					Text::Text(" ").show();
					ImGui::TableNextColumn();
					Text::Text("").show();
					ImGui::TableNextColumn();
					Text::Text("").show();
					continue;
				}

				// getting info about the current row
				bool obscure = false;
				const auto codeSectionRow = m_codeSectionController->getRow(rowIdx);
				const auto pcodeBlock = m_codeSectionController->m_imageDec->getPCodeGraph()->getBlockAtOffset(
					codeSectionRow.m_fullOffset);
				if (!pcodeBlock && m_obscureUnknownLocation) {
					obscure = true;
				}

				// select rows
				if (const auto emulator = m_projectPanel->getEmulator()) {
					if (emulator->m_imageDec == m_codeSectionController->m_imageDec) {
						bool isSelected;
						if (m_codeSectionController->m_showPCode)
							isSelected = codeSectionRow.m_isPCode && emulator->m_offset == codeSectionRow.getOffset();
						else isSelected = emulator->m_offset.getByteOffset() == codeSectionRow.getOffset().getByteOffset();
						if (isSelected) {
							m_selectCurRow = true;
							m_selectedRowColor = 0x420001FF;
						}
					}
				}
				int selRowIdx = 0;
				for (auto selRow : m_selectedRows) {
					if (m_codeSectionController->m_showPCode
						    ? selRow == codeSectionRow
						    : selRow.m_byteOffset == codeSectionRow.m_byteOffset) {
						// select func. graph
						if (selRowIdx == 0 && pcodeBlock) {
							m_pcodeGraphChanged = m_selectedFuncPCodeGraph != pcodeBlock->m_funcPCodeGraph;
							m_selectedFuncPCodeGraph = pcodeBlock->m_funcPCodeGraph;
						}
						m_selectCurRow = true;
						break;
					}
					selRowIdx++;
				}

				if (obscure)
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.7f);

				ImVec2 startRowPos;
				if (!codeSectionRow.m_isPCode) {
					const auto instrOffset = codeSectionRow.m_byteOffset;
					ImGui::TableNextColumn();
					startRowPos = ImGui::GetCursorScreenPos();
					const auto isBpSet = m_codeSectionController->m_imageDec->hasBreakpoint(codeSectionRow.m_byteOffset);
					RenderAddress(instrOffset, isBpSet);

					// Asm
					InstructionViewInfo instrViewInfo;
					std::vector<uint8_t> buffer(100);
					m_codeSectionController->m_imageDec->getImage()->read(instrOffset, buffer);
					if (m_instructionViewDecoder->decode(buffer, &instrViewInfo)) {
						CodeSectionInstructionViewer instructionViewer(&instrViewInfo);
						instructionViewer.renderJmpArrow([&]()
							{
								renderJmpLines(rowIdx, m_clipper);
							});
						instructionViewer.show();
					}
				}
				else {
					// PCode
					const auto instrOffset = codeSectionRow.m_fullOffset;
					if (const auto instr = m_codeSectionController->m_imageDec->getInstrPool()->getPCodeInstructionAt(
						instrOffset)) {
						ImGui::TableNextColumn();
						startRowPos = ImGui::GetCursorScreenPos();
						Text::Text("").show();
						ImGui::TableNextColumn();
						Text::Text("").show();
						ImGui::TableNextColumn();
						PCodeInstructionRender instrRender;
						instrRender.generate(instr);
					}
				}

				// decorate the current row
				const auto rowSize = ImVec2(tableSize.x - 25, m_clipper.ItemsHeight);
				decorateRow(startRowPos, rowSize, codeSectionRow, rowIdx, pcodeBlock);

				if (obscure)
					ImGui::PopStyleVar();
			}
		}

		// select and scroll
		if(m_scrollToRowIdx != -1) {
			m_selectedRows = { m_codeSectionController->getRow(m_scrollToRowIdx) };
		}
		scroll();
		
		ImGui::EndTable();
	}
	ImGui::EndChild();

	Show(m_builtinWindow);
	Show(m_popupModalWindow);
	Show(m_ctxWindow);
}

void GUI::ImageContentViewerPanel::CodeSectionViewer::decorateRow(const ImVec2& startRowPos, const ImVec2& rowSize,
                                                                  CodeSectionRow row, int rowIdx,
                                                                  CE::Decompiler::PCodeBlock* pcodeBlock) {
	// function header
	bool isEventProcessed = false;
	if (!row.m_isPCode) {
		if (const auto function = m_codeSectionController->m_imageDec->getFunctionAt(row.m_byteOffset)) {
			renderFunctionHeader(function, startRowPos, startRowPos + ImVec2(rowSize.x, 0), isEventProcessed);
		}
	}

	// process events
	if (!isEventProcessed && ImGui::IsWindowHovered()) {
		ImGuiContext& g = *GImGui;
		if (ImGui::IsMousePosValid(&g.IO.MousePos)) {
			if (ImRect(startRowPos, startRowPos + rowSize).Contains(g.IO.MousePos)) {
				if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
					// row's multi-selection
					if (m_startSelectionRowIdx != -1) {
						auto curRowIdx = m_startSelectionRowIdx;
						auto lastRowIdx = rowIdx;
						if (curRowIdx > lastRowIdx)
							std::swap(curRowIdx, lastRowIdx);
						m_selectedRows.clear();
						while (curRowIdx <= lastRowIdx)
							m_selectedRows.push_back(m_codeSectionController->getRow(curRowIdx++));
					}
					else {
						m_startSelectionRowIdx = rowIdx;
					}
				}
				else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
					m_startSelectionRowIdx = -1;
				}
				else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					m_selectedRows = {row};
					m_clickedPCodeBlock = pcodeBlock;
				}
				else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
					if (m_selectedRows.size() == 1)
						m_selectedRows = {row};
					delete m_ctxWindow;
					m_ctxWindow = new PopupContextWindow(new RowContextPanel(this, row));
					m_ctxWindow->open();
				}
			}
		}
	}
}

void GUI::ImageContentViewerPanel::CodeSectionViewer::renderFunctionHeader(CE::Function* function,
                                                                           const ImVec2& startRowPos,
                                                                           const ImVec2& endRowPos,
                                                                           bool& isEventProcessed) {
	const auto Color = ToImGuiColorU32(0xc9c59fFF);
	m_window->DrawList->AddLine(startRowPos, endRowPos, Color, 0.7f);

	// function name
	{
		const auto text = "func";
		const auto textSize = ImGui::CalcTextSize(text);
		const auto textPos = endRowPos - ImVec2(textSize.x, 0.0);

		// click event
		ImGuiContext& g = *GImGui;
		if (ImGui::IsMousePosValid(&g.IO.MousePos)) {
			if (ImRect(textPos, textPos + textSize).Contains(g.IO.MousePos)) {
				ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					delete m_builtinWindow;
					m_builtinWindow = new PopupBuiltinWindow(new FunctionReferencesPanel(function, this));
					m_builtinWindow->getSize() = ImVec2(300.0f, 200.0f);
					m_builtinWindow->getPos() = endRowPos - ImVec2(m_builtinWindow->getSize().x, 0);
					m_builtinWindow->addFlags(ImGuiWindowFlags_AlwaysAutoResize, false);
					m_builtinWindow->open();
					isEventProcessed = true;
				}
			}
		}

		// render
		m_window->DrawList->AddText(textPos, Color, text);
	}
}

void GUI::ImageContentViewerPanel::CodeSectionViewer::
renderJmpLines(const int rowIdx, const ImGuiListClipper& clipper) {
	if (const auto it = m_codeSectionController->m_offsetToJmp.find(
			m_codeSectionController->getRow(rowIdx).m_fullOffset);
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

void GUI::ImageContentViewerPanel::CodeSectionViewer::drawJmpLine(const int rowIdx,
                                                                  const CodeSectionControllerX86::Jmp* pJmp,
                                                                  const ImGuiListClipper& clipper) const {
	const float JmpLineLeftOffset = 10.0f;
	const float JmpLineGap = 8.0f;

	const bool isStart = m_codeSectionController->getRow(rowIdx).m_fullOffset == pJmp->m_startOffset;
	const auto targetRowIdx = m_codeSectionController->getRowIdx(
		CodeSectionRow(isStart ? pJmp->m_endOffset : pJmp->m_startOffset));

	auto lineColor = ToImGuiColorU32(-1);
	const ImVec2 point1 = {
		ImGui::GetItemRectMin().x - 2.0f, (ImGui::GetItemRectMin().y + ImGui::GetItemRectMax().y) / 2.0f
	};
	const ImVec2 point2 = ImVec2(point1.x - pJmp->m_level * JmpLineGap - JmpLineLeftOffset, point1.y);
	const ImVec2 point3 = ImVec2(point2.x, point2.y + clipper.ItemsHeight * (targetRowIdx - rowIdx));
	const ImVec2 point4 = ImVec2(point1.x, point3.y);

	// click event of the jump arrow
	ImGuiContext& g = *GImGui;
	if (ImGui::IsMousePosValid(&g.IO.MousePos)) {
		auto rect = ImRect(point2, {point3.x + JmpLineGap, point3.y});
		if (rect.Min.x > rect.Max.x || rect.Min.y > rect.Max.y)
			rect = ImRect(point3, {point2.x + JmpLineGap, point2.y});
		if (rect.Contains(g.IO.MousePos)) {
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			lineColor = ToImGuiColorU32(0x00bfffFF);
			if (ImGui::IsMouseClicked(0)) {
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

void GUI::ImageContentViewerPanel::goToOffset(CE::Offset offset) {
	const auto section = m_imageDec->getImage()->getSectionByOffset(offset);
	if (section->m_type == CE::ImageSection::NONE_SEGMENT)
		throw WarningException("Offset not found.");
	selectImageSection(section);
	m_imageSectionViewer->goToOffset(offset);
	m_projectPanel->addVisitedLocation(m_imageDec, offset);
}

void GUI::ImageContentViewerPanel::decompile(CE::Decompiler::FunctionPCodeGraph* functionPCodeGraph) {
	using namespace CE::Decompiler;

	RegisterFactoryX86 registerFactoryX86;
	const auto funcOffset = functionPCodeGraph->getStartBlock()->getMinOffset().getByteOffset();
	if (const auto function = m_imageDec->getFunctionAt(funcOffset)) {
		const auto project = m_imageDec->getImageManager()->getProject();

		if (m_decompilerStep >= DecompilerStep::DECOMPILING) {
			auto funcCallInfoCallback = [&](Instruction* instr, int offset)
			{
				if (const auto func = m_imageDec->getFunctionAt(offset)) {
					return func->getSignature()->getCallInfo();
				}
				const auto it = m_imageDec->getVirtFuncCalls().find(instr->getOffset());
				if (it != m_imageDec->getVirtFuncCalls().end())
					return it->second->getCallInfo();
				return project->getTypeManager()->getDefaultFuncSignature()->getCallInfo();
			};
			SdaCodeGraph* sdaCodeGraph = nullptr;
			auto decCodeGraph = new DecompiledCodeGraph(functionPCodeGraph);
			auto primaryDecompiler = new PrimaryDecompiler(decCodeGraph, &registerFactoryX86,
			                                               function->getSignature()->getCallInfo().getReturnInfo(),
			                                               funcCallInfoCallback);
			primaryDecompiler->start();

			if (m_decompilerStep >= DecompilerStep::PROCESSING) {
				using namespace magic_enum::bitwise_operators;
				{
					decCodeGraph->cloneAllExpr();

					if ((m_processingStep | ProcessingStep::CONDITION_PROCESSING) == m_processingStep) {
						Optimization::GraphCondBlockOptimization graphCondBlockOptimization(decCodeGraph);
						graphCondBlockOptimization.start();
					}

					if ((m_processingStep | ProcessingStep::EXPR_OPTIMIZING) == m_processingStep) {
						Optimization::GraphExprOptimization graphExprOptimization(decCodeGraph);
						graphExprOptimization.start();
					}

					if ((m_processingStep | ProcessingStep::PAR_ASSIGNMENT_CREATING) == m_processingStep) {
						Optimization::GraphParAssignmentCreator graphParAssignmentCreator(
							decCodeGraph, primaryDecompiler);
						graphParAssignmentCreator.start();
					}

					if ((m_processingStep | ProcessingStep::ORDER_FIXING) == m_processingStep) {
						Optimization::GraphLastLineAndConditionOrderFixing graphLastLineAndConditionOrderFixing(
							decCodeGraph);
						graphLastLineAndConditionOrderFixing.start();
					}

					if ((m_processingStep | ProcessingStep::VIEW_OPTIMIZING) == m_processingStep) {
						Optimization::GraphViewOptimization graphViewOptimization(decCodeGraph);
						graphViewOptimization.start();
					}

					if ((m_processingStep | ProcessingStep::DEBUG_PROCESSING) == m_processingStep) {
						Optimization::GraphDebugProcessing graphDebugProcessing(decCodeGraph, false);
						graphDebugProcessing.start();
					}

					if ((m_processingStep | ProcessingStep::LINE_EXPANDING) == m_processingStep) {
						Optimization::GraphLinesExpanding graphLinesExpanding(decCodeGraph);
						graphLinesExpanding.start();
					}

					if ((m_processingStep | ProcessingStep::DEBUG_PROCESSING) == m_processingStep) {
						Optimization::GraphDebugProcessing graphDebugProcessing(decCodeGraph, true);
						graphDebugProcessing.start();
					}

					if ((m_processingStep | ProcessingStep::USELESS_LINES_REMOVING) == m_processingStep) {
						Optimization::GraphUselessLineDeleting graphUselessLineDeleting(decCodeGraph);
						graphUselessLineDeleting.start();
					}

					DecompiledCodeGraph::CalculateHeightForDecBlocks(decCodeGraph->getStartBlock());
				}

				if (m_decompilerStep >= DecompilerStep::SYMBOLIZING) {
					if (m_symbolizingStep >= SymbolizingStep::BUILDING) {
						sdaCodeGraph = new SdaCodeGraph(decCodeGraph);
						auto symbolCtx = function->getSymbolContext();
						Symbolization::SdaBuilding sdaBuilding(sdaCodeGraph, &symbolCtx, project);
						sdaBuilding.start();

						if (m_symbolizingStep >= SymbolizingStep::CALCULATING) {
							Symbolization::SdaDataTypesCalculater sdaDataTypesCalculater(
								sdaCodeGraph, symbolCtx.m_signature, project);
							sdaDataTypesCalculater.start();
						}

						if (m_decompilerStep >= DecompilerStep::FINAL_PROCESSING) {
							if ((m_finalProcessingStep | FinalProcessingStep::MEMORY_OPTIMIZING) ==
								m_finalProcessingStep) {
								Optimization::SdaGraphMemoryOptimization memoryOptimization(sdaCodeGraph);
								memoryOptimization.start();
							}

							if ((m_finalProcessingStep | FinalProcessingStep::USELESS_LINES_OPTIMIZING) ==
								m_finalProcessingStep) {
								Optimization::SdaGraphUselessLineOptimization uselessLineOptimization(sdaCodeGraph);
								uselessLineOptimization.start();
							}
						}
					}
				}
			}

			decCodeGraph->removeNotUsedSymbols();

			// open the window
			DecompiledCodeViewerPanel* panel;
			if (sdaCodeGraph) {
				panel = new DecompiledCodeViewerPanel(m_projectPanel, sdaCodeGraph, function);
			}
			else {
				panel = new DecompiledCodeViewerPanel(m_projectPanel, decCodeGraph);
			}
			m_curDecGraph = decCodeGraph;
			panel->m_decompiledCodeViewer->setInfoToShowAsm(m_imageDec->getImage(), new InstructionViewDecoderX86);
			panel->m_decompiledCodeViewer->setInfoToShowExecCtxs(primaryDecompiler);
			panel->m_decompiledCodeViewer->m_childName = "dec_func_0x" + Helper::String::NumberToHex(function->getOffset());
			if (m_decompiledCodeViewerWindow) {
				if (auto prevPanel = dynamic_cast<DecompiledCodeViewerPanel*>(m_decompiledCodeViewerWindow->getPanel()))
					panel->m_decompiledCodeViewer->m_show = prevPanel->m_decompiledCodeViewer->m_show;
			}
			delete m_decompiledCodeViewerWindow;
			m_decompiledCodeViewerWindow = panel->createStdWindow();

			if (const auto emulator = m_projectPanel->getEmulator()) {
				emulator->updateByDecGraph(decCodeGraph);
			}
		}
	}
}

void GUI::ImageContentViewerPanel::renderMenuBar() {
	auto codeSectionViewer = dynamic_cast<CodeSectionViewer*>(m_imageSectionViewer);

	if (ImGui::BeginMenu("Select")) {
		m_imageSectionMenuListView.show();
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Navigation")) {
		if (ImGui::MenuItem("Go To")) {
			delete m_popupModalWindow;
			m_popupModalWindow = new PopupModalWindow(new GoToPanel(this));
			m_popupModalWindow->open();
		}

		if (const auto codeSectionViewer = dynamic_cast<CodeSectionViewer*>(m_imageSectionViewer)) {
			if(!codeSectionViewer->m_selectedRows.empty()) {
				if (ImGui::MenuItem("Go To Selected Row")) {
					try {
						const auto row = *codeSectionViewer->m_selectedRows.begin();
						codeSectionViewer->goToOffset(row.m_byteOffset);
					}
					catch (WarningException&) {}
				}
			}
		}
		ImGui::EndMenu();
	}

	if (codeSectionViewer) {
		if (ImGui::BeginMenu("Debug")) {
			if (codeSectionViewer->m_selectedFuncPCodeGraph) {
				if (ImGui::MenuItem("Start Emulator")) {
					m_projectPanel->createEmulator(
						m_imageDec, codeSectionViewer->m_selectedFuncPCodeGraph->getStartBlock()->getMinOffset(),
						PCodeEmulator::PCodeStepWidth::STEP_ORIGINAL_INSTR);
				}
			}
			if (const auto emulator = m_projectPanel->getEmulator(false)) {
				emulator->renderDebugMenu();
			}
			ImGui::EndMenu();
		}

		if (codeSectionViewer->m_selectedFuncPCodeGraph) {
			if (ImGui::BeginMenu("Decompiler")) {
				if (ImGui::MenuItem("Decompile")) {
					decompile(codeSectionViewer->m_selectedFuncPCodeGraph);
				}

				ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
				if (ImGui::MenuItem("Decompiling Step", nullptr, m_decompilerStep >= DecompilerStep::DECOMPILING)) {
					m_decompilerStep = DecompilerStep::DECOMPILING;
				}
				if (ImGui::MenuItem("Processing Step", nullptr, m_decompilerStep >= DecompilerStep::PROCESSING)) {
					m_decompilerStep = DecompilerStep::PROCESSING;
				}
				if (ImGui::MenuItem("Symbolizing Step", nullptr, m_decompilerStep >= DecompilerStep::SYMBOLIZING)) {
					m_decompilerStep = DecompilerStep::SYMBOLIZING;
				}
				if (ImGui::MenuItem("Final Processing Step", nullptr,
				                    m_decompilerStep >= DecompilerStep::FINAL_PROCESSING)) {
					m_decompilerStep = DecompilerStep::FINAL_PROCESSING;
				}

				if (ImGui::BeginMenu("Processing Steps")) {
					using namespace magic_enum::bitwise_operators;
					if (ImGui::MenuItem("Condition Processing Step", nullptr,
					                    (m_processingStep | ProcessingStep::CONDITION_PROCESSING) ==
					                    m_processingStep)) {
						m_processingStep ^= ProcessingStep::CONDITION_PROCESSING;
					}
					if (ImGui::MenuItem("Expr. Optimizing Step", nullptr,
					                    (m_processingStep | ProcessingStep::EXPR_OPTIMIZING) == m_processingStep)) {
						m_processingStep ^= ProcessingStep::EXPR_OPTIMIZING;
					}
					if (ImGui::MenuItem("Par. Assignment Creating Step", nullptr,
					                    (m_processingStep | ProcessingStep::PAR_ASSIGNMENT_CREATING) ==
					                    m_processingStep)) {
						m_processingStep ^= ProcessingStep::PAR_ASSIGNMENT_CREATING;
					}
					if (ImGui::MenuItem("Order Fixing Step", nullptr,
					                    (m_processingStep | ProcessingStep::ORDER_FIXING) == m_processingStep)) {
						m_processingStep ^= ProcessingStep::ORDER_FIXING;
					}
					if (ImGui::MenuItem("View Optimizing Step", nullptr,
					                    (m_processingStep | ProcessingStep::VIEW_OPTIMIZING) == m_processingStep)) {
						m_processingStep ^= ProcessingStep::VIEW_OPTIMIZING;
					}
					if (ImGui::MenuItem("Line Expanding Step", nullptr,
					                    (m_processingStep | ProcessingStep::LINE_EXPANDING) == m_processingStep)) {
						m_processingStep ^= ProcessingStep::LINE_EXPANDING;
					}
					if (ImGui::MenuItem("Useless Lines Removing Step", nullptr,
					                    (m_processingStep | ProcessingStep::USELESS_LINES_REMOVING) ==
					                    m_processingStep)) {
						m_processingStep ^= ProcessingStep::USELESS_LINES_REMOVING;
					}
					if (ImGui::MenuItem("Debug Processing Step", nullptr,
					                    (m_processingStep | ProcessingStep::DEBUG_PROCESSING) == m_processingStep)) {
						m_processingStep ^= ProcessingStep::DEBUG_PROCESSING;
					}
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Symbolizing Steps")) {
					if (ImGui::MenuItem("Building step", nullptr, m_symbolizingStep >= SymbolizingStep::BUILDING)) {
						m_symbolizingStep = SymbolizingStep::BUILDING;
					}
					if (ImGui::MenuItem("Calculating step", nullptr,
					                    m_symbolizingStep >= SymbolizingStep::CALCULATING)) {
						m_symbolizingStep = SymbolizingStep::CALCULATING;
					}
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Final Processing Steps")) {
					using namespace magic_enum::bitwise_operators;
					if (ImGui::MenuItem("Memory Optimizing Step", nullptr,
					                    (m_finalProcessingStep | FinalProcessingStep::MEMORY_OPTIMIZING) ==
					                    m_finalProcessingStep)) {
						m_finalProcessingStep ^= FinalProcessingStep::MEMORY_OPTIMIZING;
					}
					if (ImGui::MenuItem("Useless Lines Optimizing Step", nullptr,
					                    (m_finalProcessingStep | FinalProcessingStep::USELESS_LINES_OPTIMIZING) ==
					                    m_finalProcessingStep)) {
						m_finalProcessingStep ^= FinalProcessingStep::USELESS_LINES_OPTIMIZING;
					}
					ImGui::EndMenu();
				}
				ImGui::PopItemFlag();
				ImGui::EndMenu();
			}
		}
	}

	if (ImGui::BeginMenu("View")) {
		if (ImGui::MenuItem("Update")) {
			m_imageSectionViewer->m_sectionController->update();
		}
		if (codeSectionViewer) {
			if (ImGui::MenuItem("Show PCode", nullptr, codeSectionViewer->m_codeSectionController->m_showPCode)) {
				codeSectionViewer->m_codeSectionController->m_showPCode ^= true;
				codeSectionViewer->m_codeSectionController->update();
			}
			if (codeSectionViewer->m_selectedFuncPCodeGraph) {
				if (ImGui::MenuItem("Show function graph", nullptr)) {
					if (!m_funcGraphViewerWindow) {
						auto funcGraphViewerPanel = new FuncGraphViewerPanel(m_imageDec, new InstructionViewDecoderX86);
						funcGraphViewerPanel->setFuncGraph(codeSectionViewer->m_selectedFuncPCodeGraph);
						m_funcGraphViewerWindow = funcGraphViewerPanel->createStdWindow();
					}
					else {
						if (auto funcGraphViewerPanel = dynamic_cast<FuncGraphViewerPanel*>(m_funcGraphViewerWindow->
							getPanel())) {
							funcGraphViewerPanel->setFuncGraph(codeSectionViewer->m_selectedFuncPCodeGraph);
						}
					}
				}
			}
			if (ImGui::MenuItem("Obscure unknown location", nullptr, codeSectionViewer->m_obscureUnknownLocation)) {
				codeSectionViewer->m_obscureUnknownLocation ^= true;
			}
		}
		ImGui::EndMenu();
	}
}

void GUI::ImageContentViewerPanel::selectImageSection(const CE::ImageSection* imageSection) {
	delete m_imageSectionViewer;
	m_imageSectionMenuListView.m_selectedItem = imageSection;

	AbstractSectionController* controller = nullptr;
	if (const auto it = m_imageSectionControllers.find(imageSection);
		it != m_imageSectionControllers.end())
		controller = it->second;

	if (imageSection->m_type == CE::ImageSection::CODE_SEGMENT) {
		auto codeController = dynamic_cast<CodeSectionControllerX86*>(controller);
		if (!codeController)
			m_imageSectionControllers[imageSection] = codeController = new CodeSectionControllerX86(
				m_imageDec, imageSection);
		m_imageSectionViewer = new CodeSectionViewer(m_projectPanel, codeController, new InstructionViewDecoderX86);
	}
	else {
		auto dataController = dynamic_cast<DataSectionController*>(controller);
		if (!dataController)
			m_imageSectionControllers[imageSection] = dataController = new DataSectionController(
				m_imageDec, imageSection);
		m_imageSectionViewer = new DataSectionViewer(m_projectPanel, dataController);
	}
}

void GUI::ImageContentViewerPanel::processSectionViewerEvents() {
	if (const auto codeSectionViewer = dynamic_cast<CodeSectionViewer*>(m_imageSectionViewer)) {
		const auto decCodeViewerPanel = m_decompiledCodeViewerWindow
			                                ? dynamic_cast<DecompiledCodeViewerPanel*>(m_decompiledCodeViewerWindow->
				                                getPanel())
			                                : nullptr;

		// create emulator that will start on selected row
		if (codeSectionViewer->m_createEmulator) {
			if (!codeSectionViewer->m_selectedRows.empty()) {
				const auto firstRow = *codeSectionViewer->m_selectedRows.begin();
				const auto stepWidth = firstRow.m_isPCode
					? PCodeEmulator::PCodeStepWidth::STEP_PCODE_INSTR
					: PCodeEmulator::PCodeStepWidth::STEP_ORIGINAL_INSTR;
				m_projectPanel->createEmulator(m_imageDec, firstRow.getOffset(), stepWidth);
				return;
			}
			codeSectionViewer->m_createEmulator = false;
		}

		if (decCodeViewerPanel) {
			auto& instrs = decCodeViewerPanel->m_decompiledCodeViewer->m_selectedCodeByInstr;
			
			// select row that new function graph is on
			if(codeSectionViewer->m_pcodeGraphChanged) {
				decompile(codeSectionViewer->m_selectedFuncPCodeGraph);
				return;
			}
			// select block list by instruction selection
			if (codeSectionViewer->m_clickedPCodeBlock && m_curDecGraph) {
				CE::Decompiler::DecBlock* selDecBlock = nullptr;
				for (const auto decBlock : m_curDecGraph->getDecompiledBlocks()) {
					if (decBlock->m_pcodeBlock == codeSectionViewer->m_clickedPCodeBlock) {
						selDecBlock = decBlock;
						break;
					}
				}
				if (selDecBlock) {
					decCodeViewerPanel->m_decompiledCodeViewer->selectBlockList(selDecBlock);
				}
				instrs.clear();
			}
			// select dec. code by row's multi-selection
			if (codeSectionViewer->isRowsMultiSelecting()) {
				instrs.clear();
				for (const auto row : codeSectionViewer->m_selectedRows) {
					if (row.m_isPCode) {
						if (const auto instr = m_imageDec->getInstrPool()->getPCodeInstructionAt(row.getOffset())) {
							instrs.insert(instr);
						}
					}
					else {
						if (const auto origInstr = m_imageDec->getInstrPool()->getOrigInstructionAt(row.m_byteOffset)) {
							for (auto& [orderId, instr] : origInstr->m_pcodeInstructions)
								instrs.insert(&instr);
						}
					}
				}
			}
		}

		// set current location
		if (!codeSectionViewer->m_selectedRows.empty()) {
			m_projectPanel->m_curLocation = std::pair(m_imageDec, codeSectionViewer->m_selectedRows.begin()->m_byteOffset);
		}
	}
	else if (const auto dataSectionViewer = dynamic_cast<DataSectionViewer*>(m_imageSectionViewer)) {

	}
}

void GUI::ImageContentViewerPanel::processDecompiledCodeViewerEvents() {
	if (!m_decompiledCodeViewerWindow)
		return;

	if (const auto decCodeViewerPanel = dynamic_cast<DecompiledCodeViewerPanel*>(m_decompiledCodeViewerWindow->
		getPanel())) {

		if (const auto codeSectionViewer = dynamic_cast<CodeSectionViewer*>(m_imageSectionViewer)) {
			// symbol was changed
			if (decCodeViewerPanel->m_decompiledCodeViewer->m_codeChanged) {
				decompile(codeSectionViewer->m_selectedFuncPCodeGraph);
				return;
			}
			// select instructions by code selection
			if (!decCodeViewerPanel->m_decompiledCodeViewer->m_selectedInstrs.empty()) {
				codeSectionViewer->m_selectedRows.clear();
				const auto instrs = decCodeViewerPanel->m_decompiledCodeViewer->m_selectedInstrs;
				for (const auto instr : instrs) {
					codeSectionViewer->m_selectedRows.emplace_back(instr->getOffset(), true);
				}
			}
		}

		// click function
		if (const auto function = decCodeViewerPanel->m_decompiledCodeViewer->m_clickedFunction) {
			goToOffset(function->getOffset());
		}
		// click global var
		else if (const auto globalVar = decCodeViewerPanel->m_decompiledCodeViewer->m_clickedGlobalVar) {
			goToOffset(globalVar->getOffset());
		}
		// start debug
		else if (decCodeViewerPanel->m_startDebug) {
			m_projectPanel->createEmulator(m_imageDec, m_curDecGraph->getStartBlock()->m_pcodeBlock->getMinOffset(),
				PCodeEmulator::PCodeStepWidth::STEP_CODE_LINE);
		}
	}
}
