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

void GUI::PCodeEmulator::updateSymbolValuesByDecGraph(CE::Decompiler::DecompiledCodeGraph* decGraph,
                                                      CE::ComplexOffset offset, bool after) {
	const auto& symbolValues = decGraph->getSymbolValues();
	const auto it = symbolValues.find(offset);
	if (it != symbolValues.end()) {
		const auto stackFrame = getCurStackFrameInfo();
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
				const auto it2 = stackFrame->m_symbolValueMap.find(symbolHash);
				if (it2 != stackFrame->m_symbolValueMap.end()) {
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
				const auto reg = CE::Decompiler::Register(storage.getRegId(), regMask);

				if (userDefined) {
					m_execCtx.setRegisterValue(reg, userValue);
				} else {
					CE::Decompiler::DataValue value;
					if (m_execCtx.getRegisterValue(reg, value)) {
						stackFrame->m_symbolValueMap[symbolHash] = { value >> storage.getOffset(), false };
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
							stackFrame->m_symbolValueMap[symbolHash] = { value, false };
						}
					}
				}
			} else {
				// memVar
				if (userDefined) {
					if(const auto outputVarnode = stackFrame->m_lastExecutedInstr->m_output) {
						m_execCtx.setValue(outputVarnode, userValue);
					}
				}
				else {
					stackFrame->m_symbolValueMap[symbolHash] = { stackFrame->m_lastExecutedInstrValue, false };
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

bool GUI::PCodeEmulator::defineCurPCodeInstruction() {
	m_curInstr = m_imageDec->getInstrPool()->getPCodeInstructionAt(m_offset);
	if (m_curInstr) {
		m_curPCodeBlock = m_imageDec->getPCodeGraph()->getBlockAtOffset(m_offset);
	}
	else {
		m_curPCodeBlock = nullptr;
		m_curBlockTopNode = nullptr;
		m_curDecGraph = nullptr;
	}

	if (m_locationHandler.isInit()) {
		const auto newAddr = m_imageDec->getImage()->getAddress() + m_offset.getByteOffset();
		const auto delta = std::abs(static_cast<int64_t>(m_curAddr) - static_cast<int64_t>(newAddr));
		m_curAddr = newAddr;
		m_locationHandler(delta);

		// dec. graph
		if (m_curInstr && m_curDecGraph) {
			m_curBlockTopNode = m_curDecGraph->findBlockTopNodeAtOffset(m_offset);
			m_relStackPointerValue = m_curDecGraph->getStackPointerValueAtOffset(m_offset - 1);

			// update symbol values
			const auto stackFrame = getCurStackFrameInfo();
			if (stackFrame->m_lastExecutedInstr) {
				const auto instrOffset = stackFrame->m_lastExecutedInstr->getOffset();
				// after the last executed instruction
				updateSymbolValuesByDecGraph(m_curDecGraph, instrOffset, true);
			}
			// before the current instruction
			updateSymbolValuesByDecGraph(m_curDecGraph, m_offset, false);
		}
	}

	if (!m_curInstr) {
		// there may not be PCode instruction in real debugging
		m_isStopped = true;
		return false;
	}
	return true;
}

void GUI::PCodeEmulator::stepNextPCodeInstr() {
	// execute the current instruction
	m_vm.execute(m_curInstr);

	// stack frame
	if (m_curDecGraph) {
		m_relStackPointerValue = m_curDecGraph->getStackPointerValueAtOffset(m_curInstr->getOffset());
		const auto stackFrameInfo = getCurStackFrameInfo();
		if (m_curInstr->m_id == CE::Decompiler::InstructionId::RETURN) {
			m_stackFrameInfo.erase(getCurrentStackFrame());
		}
		stackFrameInfo->m_lastExecutedInstr = m_curInstr;
		stackFrameInfo->m_lastExecutedInstrValue = m_vm.m_result;
	}

	// go to the next instruction
	defineNextInstrOffset();
	if (!isWorking())
		return;
	if (isNewOrigInstruction()) {
		sync();
	}
	else {
		defineCurPCodeInstruction();
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
