#include "Windows.h" // windows (for keys VK_F5, ...)
#include "ProjectPanel.h"

void GUI::ProjectPanel::renderPanel() {
	checkUnsavedState();
	ImGui::DockSpace(m_dockSpaceId = ImGui::GetID("ProjectDockSpace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

	m_imageContentWinManager.m_dockSpaceId = m_dockSpaceId;
	m_imageContentWinManager.show();
	Show(m_imageViewerWindow);
	Show(m_funcViewerWindow);
	Show(m_dataTypeViewerWindow);
	Show(m_symbolViewerWindow);
	Show(m_debugAttachProcessWindow);
	Show(m_messageWindow);

	// debugger & emulator
	if (m_debugger) {
		if (m_debugger->isWorking()) {
			m_debugger->show();
		}
		else {
			// remove debugger and all related windows
			for (const auto& [addr, imageDec] : m_debugger->m_images) {
				const auto window = getImageContentViewerWindow(imageDec);
				m_imageContentWinManager.removeWindow(window);
				delete window;
			}
			delete m_debugger;
			m_debugger = nullptr;
		}
	}
	if (const auto emulator = getEmulator(false)) {
		emulator->show();
	}

	// location history to move across
	if (ImGui::IsKeyDown(VK_MENU)) { // todo: linux support
		auto goToNewOffset = false;
		if(ImGui::IsKeyPressed(VK_RIGHT)) {
			if(m_visitedLocationIdx <= static_cast<int>(m_visitedLocationsHistory.size()) - 3) {
				m_visitedLocationIdx++;
				goToNewOffset = true;
			}
		}
		else if (ImGui::IsKeyPressed(VK_LEFT)) {
			if (m_visitedLocationIdx >= 0) {
				m_visitedLocationIdx--;
				goToNewOffset = true;
			}
		}

		if (goToNewOffset) {
			const auto& [imageDec, offset] = m_visitedLocationsHistory[m_visitedLocationIdx + 1];
			if (const auto window = getImageContentViewerWindow(imageDec)) {
				if (const auto panel = dynamic_cast<ImageContentViewerPanel*>(window->getPanel())) {
					try {
						m_lockLocationHistoryUpdate = true;
						panel->goToOffset(offset);
						m_lockLocationHistoryUpdate = false;
					}
					catch (WarningException&) {}
				}
			}
		}
	}
}
