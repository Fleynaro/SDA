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
