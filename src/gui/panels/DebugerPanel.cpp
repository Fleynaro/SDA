#include "Windows.h" // windows (for keys VK_F5, ...)
#include "DebugerPanel.h"
#include "decompiler/Graph/DecCodeGraph.h"


void GUI::PCodeEmulator::updateByDecGraph(CE::Decompiler::DecompiledCodeGraph* decGraph) {
	if (m_curInstr) {
		const auto instrOffset = m_curInstr->getOffset();
		m_curBlockTopNode = decGraph->findBlockTopNodeAtOffset(instrOffset);
		m_stackPointerValue = decGraph->getStackPointerValueAtOffset(instrOffset);
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
