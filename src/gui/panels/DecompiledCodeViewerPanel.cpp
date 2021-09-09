#include "DecompiledCodeViewerPanel.h"
#include "ProjectPanel.h"

void GUI::DecompiledCodeViewerPanel::DecompiledCodeViewerWithDebugAndHeader::CodeGenerator::ExprTreeGenerator::
generateNode(CE::Decompiler::INode* node, bool& hasGroup, MultiLineGroup::Group& group) {
	DecompiledCodeViewer::CodeGenerator::ExprTreeGenerator::generateNode(node, hasGroup, group);

	const auto decCodeViewer = dynamic_cast<DecompiledCodeViewerWithDebugAndHeader*>(m_decCodeViewer);
	const auto projectPanel = decCodeViewer->m_projectPanel;
	const auto emulator = projectPanel->getEmulator();
	if (!emulator)
		return;
	if (!emulator->m_curPCodeBlock || emulator->m_curPCodeBlock->m_funcPCodeGraph != decCodeViewer->m_sdaCodeGraph->getDecGraph()->getFuncGraph())
		return;
	const auto storagePathNode = dynamic_cast<CE::Decompiler::IStoragePathNode*>(node);
	if (!storagePathNode)
		return;
	
	if (!hasGroup) {
		auto isSelected = false;
		if (emulator->m_valueViewerWin) {
			isSelected = node == m_decCodeViewer->m_debugSelectedNode;
		}

		if (isSelected)
			m_parentGen->beginSelecting(COLOR_BG_DEBUG_SELECTED_TEXT);
		m_groups.beginGroup(node);
		ExprTreeViewGenerator::generateNode(node);
		group = m_groups.endGroup();
		if (isSelected)
			m_parentGen->endSelecting();
		hasGroup = true;
	}

	if (group.m_events.isHovered() && !emulator->m_valueViewerWin) {
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		if (m_decCodeViewer->m_hoverTimer) {
			if (GetTimeInMs() - m_decCodeViewer->m_hoverTimer > 200) {
				const auto path = storagePathNode->getStoragePath();
				if (path.m_symbol) {
					std::string name = "value";
					CE::DataTypePtr dataType;
					if (const auto sdaNode = dynamic_cast<CE::Decompiler::ISdaNode*>(node)) {
						if (const auto sdaSymbolLeaf = dynamic_cast<CE::Decompiler::SdaSymbolLeaf*>(node)) {
							// don't show function address because it's useless
							if (dynamic_cast<CE::Symbol::FunctionSymbol*>(sdaSymbolLeaf->getSdaSymbol()))
								return;
							name = sdaSymbolLeaf->getSdaSymbol()->getName();
							dataType = sdaSymbolLeaf->getSdaSymbol()->getDataType();
						}
						else {
							dataType = sdaNode->getSrcDataType();
						}
					}
					else {
						const auto project = emulator->m_imageDec->getImageManager()->getProject();
						dataType = project->getTypeManager()->getDefaultType(storagePathNode->getSize());
					}

					emulator->createValueViewer(path, name, dataType);
					m_decCodeViewer->m_debugSelectedNode = node;
				}
			}
		}
		else {
			// delay for opening
			m_decCodeViewer->m_hoverTimer = GetTimeInMs();
		}

		m_decCodeViewer->m_isObjHovered = true;
	}
}

void GUI::DecompiledCodeViewerPanel::DecompiledCodeViewerWithDebugAndHeader::CodeGenerator::generateBlockTopNode(
	CE::Decompiler::DecBlock::BlockTopNode* blockTopNode, CE::Decompiler::INode* node) {
	if (blockTopNode) {
		const auto projectPanel = dynamic_cast<DecompiledCodeViewerWithDebugAndHeader*>(m_decCodeViewer)->m_projectPanel;
		if (const auto emulator = projectPanel->getEmulator()) {
			if (emulator->m_curBlockTopNode == blockTopNode) {
				m_decCodeViewer->m_debugSelectedLineIdx = m_curLineIdx;
			}
		}
	}
	CodeViewGenerator::generateBlockTopNode(blockTopNode, node);
}

void GUI::DecompiledCodeViewerPanel::renderMenuBar() {
	m_startDebug = false;

	if (ImGui::BeginMenu("Debug")) {
		if (ImGui::MenuItem("Start Emulator")) {
			m_startDebug = true;
		}
		if (const auto emulator = m_projectPanel->getEmulator())
			emulator->renderDebugMenu();
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("View")) {
		if (ImGui::MenuItem("Show All Blocks")) {
			m_decompiledCodeViewer->m_hidedBlockLists.clear();
		}

		ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
		if (ImGui::MenuItem("Show Assembler", nullptr, m_decompiledCodeViewer->m_show.Asm)) {
			m_decompiledCodeViewer->m_show.Asm ^= true;
		}
		if (m_decompiledCodeViewer->m_show.Asm) {
			if (ImGui::MenuItem("Show PCode", nullptr, m_decompiledCodeViewer->m_show.PCode)) {
				m_decompiledCodeViewer->m_show.PCode ^= true;
			}
		}
		if (ImGui::MenuItem("Show Exec. Contexts", nullptr, m_decompiledCodeViewer->m_show.ExecCtxs)) {
			m_decompiledCodeViewer->m_show.ExecCtxs ^= true;
		}
		if (ImGui::MenuItem("Show Comments", nullptr, m_decompiledCodeViewer->m_show.DebugComments)) {
			m_decompiledCodeViewer->m_show.DebugComments ^= true;
		}
		if (m_decompiledCodeViewer->m_show.DebugComments) {
			if (ImGui::MenuItem("Show All Goto", nullptr, m_decompiledCodeViewer->m_show.DebugAllGoto)) {
				m_decompiledCodeViewer->m_show.DebugAllGoto ^= true;
			}
		}
		ImGui::PopItemFlag();
		ImGui::EndMenu();
	}
}
