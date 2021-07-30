#include "DataTypeEditorPanel.h"
#include "panels/DataTypeManagerPanel.h"

void GUI::TypedefEditorPanel::renderExtra() {
	Text::Text("Reference data type:").show();
	if (Button::StdButton(m_refDataType->getDisplayName()).present()) {
		delete m_builtinWin;
		const auto panel = new DataTypeSelectorPanel(m_dataType->getTypeManager());
		panel->handler([&, panel](CE::DataTypePtr dataType)
		{
			m_refDataType = dataType;
			panel->m_window->close();
		});
		m_builtinWin = new PopupBuiltinWindow(panel);
		m_builtinWin->getPos() = GetLeftBottom();
		m_builtinWin->open();
	}

	Show(m_builtinWin);
}

void GUI::StructureEditorPanel::FieldTableListView::renderColumn(const std::string& colText, const ColInfo* colInfo,
                                                                 CE::DataType::IStructure::Field* const& field) {
	ImGui::BeginGroup();
	ImGui::PushID(field->getAbsBitOffset());
	if (ImGui::Selectable(colText.c_str(), m_structEditorPanel->m_selectedFieldOffset == field->getAbsBitOffset(),
		ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_SpanAllColumns)) {
		m_structEditorPanel->m_selectedFieldOffset = field->getAbsBitOffset();
		m_structEditorPanel->m_selectedBitFieldDataType = field->isBitField() ? field->getDataType() : nullptr;
	}
	ImGui::PopID();
	ImGui::EndGroup();

	if (field->m_isDefault)
		return;
	
	const auto events = GenericEvents(true);
	if(events.isHovered()) {
		if (colInfo->m_name != "Length")
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
	}
	if (events.isClickedByMiddleMouseBtn()) {
		if (colInfo->m_name == "Offset") {
			const auto panel = new BuiltinTextInputPanel(colText);
			panel->handler([&, panel, field](const std::string& offset)
				{
					using namespace Helper::String;
					const auto parts = Split(offset, ":");
					const auto byteOffset =
						parts[0][1] == 'x' ? static_cast<int>(HexToNumber(parts[0])) : std::stoi(parts[0]);
					auto bitOffset = 0;
					auto bitSize = field->getBitSize();
					if(parts.size() >= 2)
						bitOffset = std::stoi(parts[1]);
					if (parts.size() == 3)
						bitSize = std::stoi(parts[2]);

					m_structEditorPanel->m_fields.erase(field->getAbsBitOffset());

					const auto newAbsBitOffset = byteOffset * 0x8 + bitOffset;
					panel->m_errorMessage = "";
					if(!m_structEditorPanel->m_fields.areEmptyFields(newAbsBitOffset, bitSize)) {
						panel->m_errorMessage = "the field overlaps the next field";
						m_structEditorPanel->m_fields[field->getAbsBitOffset()] = field;
						return;
					}
				
					m_structEditorPanel->m_fields[newAbsBitOffset] = field;
					field->setAbsBitOffset(newAbsBitOffset);
					field->setBitSize(bitSize);
					panel->m_window->close();
				});
			m_structEditorPanel->createWindow(panel);
		}
		else if (colInfo->m_name == "DataType") {
			const auto panel = new DataTypeSelectorPanel(field->getManager()->getProject()->getTypeManager(), field->getDataType()->getDisplayName());
			panel->handler([&, panel, field](CE::DataTypePtr dataType)
				{
					const auto baseAbsBitOffset = field->getOffset() * 0x8;
					const auto prevBitSize = field->getSize() * 0x8;
					const auto newBitSize = dataType->getSize() * 0x8;
					const auto deltaSize = newBitSize - prevBitSize;
				
					panel->m_errorMessage = "";
					if (deltaSize != 0) {
						if (!m_structEditorPanel->m_fields.areEmptyFields(baseAbsBitOffset + prevBitSize, deltaSize)) {
							panel->m_errorMessage = "the field overlaps the next field";
							return;
						}
					}
					
					if (field->isBitField()) {
						for(int i = 0; i < prevBitSize; i ++) {
							const auto it = m_structEditorPanel->m_fields.find(baseAbsBitOffset + i);
							if(it != m_structEditorPanel->m_fields.end()) {
								it->second->setDataType(dataType);
							}
						}
					}
					else {
						field->setBitSize(dataType->getSize() * 0x8);
						field->setDataType(dataType);
					}
					// todo: not good to save here. Field symbol should be owned by IStructure::FieldMapType, no symbol manager
					field->getManager()->getProject()->getTransaction()->markAsDirty(field);
					panel->m_window->close();
				});
			m_structEditorPanel->createWindow(panel);
		}
		else if (colInfo->m_name == "Name") {
			const auto panel = new BuiltinTextInputPanel(field->getName());
			panel->handler([&, panel, field](const std::string& name)
				{
					field->setName(name);
					field->getManager()->getProject()->getTransaction()->markAsDirty(field);
					panel->m_window->close();
				});
			m_structEditorPanel->createWindow(panel);
		}
	}
}

void GUI::FuncSigEditorPanel::ParamTableListView::renderColumn(const std::string& colText, const ColInfo* colInfo,
                                                               CE::Symbol::FuncParameterSymbol* const& param) {
	ImGui::BeginGroup();
	ImGui::PushID(param);
	if (ImGui::Selectable(colText.c_str(), m_sigEditorPanel->m_selectedParam == param,
	                      ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowItemOverlap |
	                      ImGuiSelectableFlags_SpanAllColumns)) {
		m_sigEditorPanel->m_selectedParam = param;
	}
	ImGui::PopID();
	ImGui::EndGroup();

	const auto events = GenericEvents(true);
	if (events.isHovered()) {
		if (colInfo->m_name != "Index")
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
	}
	if (events.isClickedByMiddleMouseBtn()) {
		if (colInfo->m_name == "DataType") {
			const auto panel = new DataTypeSelectorPanel(param->getManager()->getProject()->getTypeManager(),
			                                             param->getDataType()->getDisplayName());
			panel->handler([&, panel, param](CE::DataTypePtr dataType)
				{
					param->setDataType(dataType);
					m_sigEditorPanel->m_params.updateParameterStorages();
					panel->m_window->close();
				});
			m_sigEditorPanel->createWindow(panel);
		}
		else if (colInfo->m_name == "Name") {
			const auto panel = new BuiltinTextInputPanel(param->getName());
			panel->handler([&, panel, param](const std::string& name)
				{
					param->setName(name);
					panel->m_window->close();
				});
			m_sigEditorPanel->createWindow(panel);
		}
	}
}
