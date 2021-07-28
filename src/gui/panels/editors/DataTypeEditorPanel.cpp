#include "DataTypeEditorPanel.h"
#include "panels/DataTypeManagerPanel.h"

void GUI::TypedefEditorPanel::renderExtra() {
	Text::Text("Reference data type:").show();
	if (Button::StdButton(m_refDataType->getDisplayName()).present()) {
		delete m_builtinWin;
		const auto panel = new DataTypeSelectorPanel(m_dataType->getTypeManager());
		panel->handler([&](CE::DataTypePtr dataType)
		{
			m_refDataType = dataType;
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
	ImGui::PushID(field);
	if (ImGui::Selectable(colText.c_str(), m_structEditorPanel->m_selectedField == field,
		ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_SpanAllColumns)) {
		m_structEditorPanel->m_selectedField = field;
	}
	ImGui::PopID();
	ImGui::EndGroup();

	const auto events = GenericEvents(true);
	if(events.isHovered()) {
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
	}
	if (events.isClickedByMiddleMouseBtn()) {
		if (colInfo->m_name == "Offset") {
			const auto panel = new BuiltinIntegerInputPanel(field->getOffset());
			panel->handler([&, field](const int& offset)
				{
					const auto newAbsBitOffset = offset * 0x8 + field->getBitOffset();
					m_structEditorPanel->m_fields.erase(field->getAbsBitOffset());
					m_structEditorPanel->m_fields[newAbsBitOffset] = field;
					field->setAbsBitOffset(newAbsBitOffset);
				});
			createWindow(panel);
		}
		else if (colInfo->m_name == "Bit Offset") {
			const auto panel = new BuiltinIntegerInputPanel(field->getBitOffset());
			panel->handler([&, field](const int& bitOffset)
				{
					const auto newAbsBitOffset = field->getOffset() * 0x8 + bitOffset;
					m_structEditorPanel->m_fields.erase(field->getAbsBitOffset());
					m_structEditorPanel->m_fields[newAbsBitOffset] = field;
					field->setAbsBitOffset(newAbsBitOffset);
				});
			createWindow(panel);
		}
		else if (colInfo->m_name == "DataType") {
			const auto panel = new DataTypeSelectorPanel(field->getManager()->getProject()->getTypeManager(), field->getDataType()->getDisplayName());
			panel->handler([&, field](CE::DataTypePtr dataType)
			{
				field->setDataType(dataType);
				// todo: not good to save here. Field symbol should be owned by IStructure::FieldMapType, no symbol manager
				field->getManager()->getProject()->getTransaction()->markAsDirty(field);
			});
			createWindow(panel);
		}
		else if (colInfo->m_name == "Name") {
			const auto panel = new BuiltinTextInputPanel(field->getName());
			panel->handler([&, field](const std::string& name)
			{
				field->setName(name);
				field->getManager()->getProject()->getTransaction()->markAsDirty(field);
			});
			createWindow(panel);
		}
	}
}
