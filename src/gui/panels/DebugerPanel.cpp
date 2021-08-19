#include "Windows.h" // windows (for keys VK_F5, ...)
#include "DebugerPanel.h"
#include "decompiler/Graph/DecCodeGraph.h"


void GUI::PCodeEmulator::updateByDecGraph(CE::Decompiler::DecompiledCodeGraph* decGraph) {
	if (m_curInstr) {
		const auto instrOffset = m_curInstr->getOffset();
		m_curBlockTopNode = decGraph->findBlockTopNodeAtOffset(instrOffset);
		m_stackPointerValue = decGraph->getStackPointerValueAtOffset(instrOffset);
		updateSymbolValuesByDecGraph(decGraph, instrOffset, false);
	}
	if(m_lastExecutedInstrOffset != CE::InvalidOffset) {
		updateSymbolValuesByDecGraph(decGraph, m_lastExecutedInstrOffset, true);
	}
}

void GUI::PCodeEmulator::updateSymbolValuesByDecGraph(CE::Decompiler::DecompiledCodeGraph* decGraph,
                                                      CE::ComplexOffset offset, bool after) {
	const auto& symbolValues = decGraph->getSymbolValues();
	const auto it = symbolValues.find(offset);
	if (it != symbolValues.end()) {
		for (const auto& [symbol, info] : it->second) {
			if (info.m_after == after)
				continue;
			const auto& storage = info.m_storage;
			if(storage.getType() == CE::Decompiler::Storage::STORAGE_REGISTER) {
				const auto regMask = CE::Decompiler::BitMask64(symbol->getSize(), storage.getOffset());
				const auto reg = CE::Decompiler::Register(storage.getRegisterId(), regMask);
				CE::Decompiler::DataValue value;
				if (m_execCtx.getRegisterValue(reg, value)) {
					m_symbolValues.top()[symbol] = value >> storage.getOffset();
				}
			}
			else if (storage.getType() == CE::Decompiler::Storage::STORAGE_STACK || storage.getType() == CE::Decompiler::Storage::STORAGE_GLOBAL) {
				CE::Decompiler::RegisterId regId;
				auto memOffset = storage.getOffset();
				if (storage.getType() == CE::Decompiler::Storage::STORAGE_STACK) {
					regId = ZYDIS_REGISTER_RSP;
					memOffset -= m_stackPointerValue;
				} else {
					regId = ZYDIS_REGISTER_RIP;
					memOffset -= offset.getByteOffset();
				}
				const auto reg = CE::Decompiler::Register(regId, 0, CE::Decompiler::BitMask64(8));
				CE::Decompiler::DataValue baseAddr;
				if (m_execCtx.getRegisterValue(reg, baseAddr)) {
					CE::Decompiler::DataValue value;
					if (m_memCtx.getValue(baseAddr + memOffset, value)) {
						m_symbolValues.top()[symbol] = value;
					}
				}
			} else {
				m_symbolValues.top()[symbol] = m_lastExecutedInstrValue;
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
