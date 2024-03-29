#pragma once
#include "controllers/SymbolManagerController.h"
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Input.h"
#include "panels/DataTypeManagerPanel.h"


namespace GUI
{
	class SymbolEditorPanel : public AbstractPanel
	{
		CE::Symbol::AbstractSymbol* m_symbol;
		Input::TextInput m_nameInput;
		CE::DataTypePtr m_dataType;
		PopupBuiltinWindow* m_builtinWin = nullptr;
	public:
		SymbolEditorPanel(CE::Symbol::AbstractSymbol* symbol)
			: AbstractPanel("Symbol Editor"), m_symbol(symbol)
		{
			m_nameInput.setInputText(symbol->getName());
			m_dataType = symbol->getDataType();
		}

		~SymbolEditorPanel() {
			delete m_builtinWin;
		}

	private:
		void renderPanel() override {
			Text::Text("Symbol name:").show();
			m_nameInput.show();
			
			Text::Text("Symbol data type:").show();
			if (Button::StdButton(m_dataType->getDisplayName()).present()) {
				delete m_builtinWin;
				const auto panel = new DataTypeSelectorPanel(m_symbol->getManager()->getProject()->getTypeManager());
				panel->handler([&, panel](CE::DataTypePtr dataType)
					{
						m_dataType = dataType;
						panel->m_window->close();
					});
				m_builtinWin = new PopupBuiltinWindow(panel);
				m_builtinWin->getPos() = GetLeftBottom();
				m_builtinWin->open();
			}

			NewLine();
			if (Button::StdButton("Save").present()) {
				save();
				m_window->close();
			}
			SameLine();
			if (Button::StdButton("Cancel").present()) {
				m_window->close();
			}

			Show(m_builtinWin);
		}

		void save() {
			m_symbol->setName(m_nameInput.getInputText());
			m_symbol->setDataType(m_dataType);
			m_symbol->getManager()->getProject()->getTransaction()->markAsDirty(m_symbol);
		}
	};
};
