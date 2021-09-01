#include "Windows.h" // windows (for keys VK_F5, ...)
#include "DebugerPanel.h"
#include "decompiler/Graph/DecCodeGraph.h"


GUI::DebuggerAttachProcessPanel::DebuggerAttachProcessPanel(CE::Project* project)
	: AbstractPanel("Attach Process"), m_processListModel(&m_debugProcesses), m_addrSpaceController(project->getAddrSpaceManager())
{
	m_selectedDebugger = *CE::GetAvailableDebuggers().begin();
	m_selectedDebuggerStr = GetDubuggerName(m_selectedDebugger);

#ifndef NDEBUG
	// for test only
	m_processListModel.m_filterName = "test";
#endif

	m_debugProcesses = CE::GetProcesses();
	m_tableProcessListView = SelectableTableListView(&m_processListModel, {
		                                                 ColInfo("PID", ImGuiTableColumnFlags_WidthFixed, 50.0f),
		                                                 ColInfo("Name", ImGuiTableColumnFlags_None)
	                                                 });
	m_tableProcessListView.handler([&](CE::DebugProcess* process)
	{
		m_selectedProcess = process;
	});

	m_addrSpaceListView = StdListView<CE::AddressSpace*>(&m_addrSpaceController.m_listModel, nullptr, true);
	m_addrSpaceListView.handler([&](CE::AddressSpace* addrSpace)
	{
		m_selectedParentAddrSpace = addrSpace;
		m_selectedParentAddrSpaceStr = addrSpace ? addrSpace->getName() : "Not selected";
	});
	m_selectedParentAddrSpaceStr = "Not selected";
	m_addrSpaceController.m_filter.m_showDebug = false;
}

void GUI::PCodeEmulator::updateByDecGraph(CE::Decompiler::DecompiledCodeGraph* decGraph) {
	if (m_lastExecutedInstr) {
		const auto instrOffset = m_lastExecutedInstr->getOffset();
		// after the last executed instruction
		updateSymbolValuesByDecGraph(decGraph, instrOffset, true);
	}
	if (m_curInstr) {
		const auto instrOffset = m_curInstr->getOffset();
		m_curBlockTopNode = decGraph->findBlockTopNodeAtOffset(instrOffset);
		m_relStackPointerValue = decGraph->getStackPointerValueAtOffset(instrOffset);
		// before the current instruction
		updateSymbolValuesByDecGraph(decGraph, instrOffset, false);
	}
}

void GUI::PCodeEmulator::updateSymbolValuesByDecGraph(CE::Decompiler::DecompiledCodeGraph* decGraph,
                                                      CE::ComplexOffset offset, bool after) {
	const auto& symbolValues = decGraph->getSymbolValues();
	const auto it = symbolValues.find(offset);
	if (it != symbolValues.end()) {
		const auto symbolValueMap = getSymbolValueMap();
		for (const auto& valueInfo : it->second) {
			if (valueInfo.m_after != after)
				continue;
			const auto symbol = valueInfo.m_symbol;
			const auto symbolHash = symbol->getHash();
			const auto& storage = valueInfo.m_storage;

			// if user changed value for the symbol then need to set user's value into context
			auto userDefined = false;
			CE::Decompiler::DataValue userValue = 0;
			{
				const auto it2 = symbolValueMap->find(symbolHash);
				if (it2 != symbolValueMap->end()) {
					if (it2->second.m_userDefined) {
						userValue = it2->second.m_value;
						it2->second.m_userDefined = false;
						userDefined = true;
					}
				}
			}

			// take value for the symbol from contexts (or set user value)
			if(storage.getType() == CE::Decompiler::Storage::STORAGE_REGISTER) {
				// param registers (rcx, rdx)
				const auto regMask = CE::Decompiler::BitMask64(symbol->getSize(), static_cast<int>(storage.getOffset()));
				const auto reg = CE::Decompiler::Register(storage.getRegisterId(), regMask);

				if (userDefined) {
					m_execCtx.setRegisterValue(reg, userValue);
				} else {
					CE::Decompiler::DataValue value;
					if (m_execCtx.getRegisterValue(reg, value)) {
						(*symbolValueMap)[symbolHash] = { value >> storage.getOffset(), false };
					}
				}
			}
			else if (storage.getType() == CE::Decompiler::Storage::STORAGE_STACK || storage.getType() == CE::Decompiler::Storage::STORAGE_GLOBAL) {
				// param memVar
				CE::Decompiler::RegisterId regId;
				auto memOffset = storage.getOffset();
				if (storage.getType() == CE::Decompiler::Storage::STORAGE_STACK) {
					regId = ZYDIS_REGISTER_RSP;
					memOffset -= m_relStackPointerValue;
				} else {
					regId = ZYDIS_REGISTER_RIP;
					memOffset -= offset.getByteOffset();
				}
				const auto reg = CE::Decompiler::Register(regId, 0, CE::Decompiler::BitMask64(8));
				
				CE::Decompiler::DataValue baseAddr;
				if (m_execCtx.getRegisterValue(reg, baseAddr)) {
					if (userDefined) {
						m_memCtx.setValue(baseAddr + memOffset, userValue);
					} else {
						CE::Decompiler::DataValue value;
						if (m_memCtx.getValue(baseAddr + memOffset, value)) {
							(*symbolValueMap)[symbolHash] = { value, false };
						}
					}
				}
			} else {
				// memVar
				if (userDefined) {
					if(const auto outputVarnode = m_lastExecutedInstr->m_output) {
						m_execCtx.setValue(outputVarnode, userValue);
					}
				}
				else {
					(*symbolValueMap)[symbolHash] = { m_lastExecutedInstrValue, false };
				}
			}
		}
	}
}

void GUI::PCodeEmulator::renderControl() {
	if (isWorking()) {
		if (ImGui::IsKeyPressed(VK_F8)) {
			// todo: linux, change imgui_impl_win32.cpp
			stepOver();
		}
		else if (ImGui::IsKeyPressed(VK_F7)) {
			stepInto();
		}
	}

	if (isWorking() || m_showContexts) {
		Show(m_execCtxViewerWin);
		Show(m_stackViewerWin);
	}
	if (isWorking()) {
		Show(m_valueViewerWin);
	}
}

void GUI::Debugger::PCodeEmulatorWithDebugSession::renderControl() {
	PCodeEmulator::renderControl();
	if (m_isExit) {
		m_debugSession->stop();
	}
	if (isWorking()) {
		if (ImGui::IsKeyPressed(VK_F5)) {
			if (m_debugSession->isSuspended()) {
				m_debugSession->resume();
			}
			else {
				m_debugSession->pause();
			}
		}
	}
}
