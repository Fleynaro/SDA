#include "Windows.h" // windows (for keys VK_F5, ...)
#include "DebugerPanel.h"
#include "decompiler/Graph/DecCodeGraph.h"


void GUI::PCodeEmulator::updateByDecGraph(CE::Decompiler::DecompiledCodeGraph* decGraph) {
	if (m_lastExecutedInstr) {
		const auto instrOffset = m_lastExecutedInstr->getOffset();
		updateSymbolValuesByDecGraph(decGraph, instrOffset, true);
	}
	if (m_curInstr) {
		const auto instrOffset = m_curInstr->getOffset();
		m_curBlockTopNode = decGraph->findBlockTopNodeAtOffset(instrOffset);
		m_relStackPointerValue = decGraph->getStackPointerValueAtOffset(instrOffset);
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
			const auto& storage = valueInfo.m_storage;

			// take value for the symbol from contexts
			if(storage.getType() == CE::Decompiler::Storage::STORAGE_REGISTER) {
				const auto regMask = CE::Decompiler::BitMask64(symbol->getSize(), static_cast<int>(storage.getOffset()));
				const auto reg = CE::Decompiler::Register(storage.getRegisterId(), regMask);

				// if user changed value for the symbol then set user's value into context
				auto userDefined = false;
				const auto it2 = symbolValueMap->find(symbol);
				if (it2 != symbolValueMap->end()) {
					if (it2->second.m_userDefined) {
						m_execCtx.setRegisterValue(reg, it2->second.m_value);
						it2->second.m_userDefined = false;
						userDefined = true;
					}
				}

				if (!userDefined) {
					CE::Decompiler::DataValue value;
					if (m_execCtx.getRegisterValue(reg, value)) {
						(*symbolValueMap)[symbol].m_value = value >> storage.getOffset();
					}
				}
			}
			else if (storage.getType() == CE::Decompiler::Storage::STORAGE_STACK || storage.getType() == CE::Decompiler::Storage::STORAGE_GLOBAL) {
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
					// if user changed value for the symbol then set user's value into context
					auto userDefined = false;
					const auto it2 = symbolValueMap->find(symbol);
					if (it2 != symbolValueMap->end()) {
						if (it2->second.m_userDefined) {
							m_memCtx.setValue(baseAddr + memOffset, it2->second.m_value);
							it2->second.m_userDefined = false;
							userDefined = true;
						}
					}

					if (!userDefined) {
						CE::Decompiler::DataValue value;
						if (m_memCtx.getValue(baseAddr + memOffset, value)) {
							addSymbolValue(symbol, value);
						}
					}
				}
			} else {
				// memVar
				addSymbolValue(symbol, m_lastExecutedInstrValue);
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
