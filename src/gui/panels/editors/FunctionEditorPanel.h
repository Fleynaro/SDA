#pragma once
#include "controllers/SymbolManagerController.h"
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Input.h"
#include "managers/FunctionManager.h"
#include "panels/DataTypeManagerPanel.h"


namespace GUI
{
	class FunctionEditorPanel : public AbstractPanel
	{
		CE::Function* m_function;
		Input::TextInput m_nameInput;
		CE::DataType::IFunctionSignature* m_funcSignature;
		PopupBuiltinWindow* m_builtinWin = nullptr;
		StdWindow* m_editorWin = nullptr;
	public:
		FunctionEditorPanel(CE::Function* function)
			: AbstractPanel("Function Editor"), m_function(function)
		{
			m_nameInput.setInputText(function->getName());
			m_funcSignature = function->getSignature();
		}

		~FunctionEditorPanel() {
			delete m_builtinWin;
			delete m_editorWin;
		}

	private:
		void renderPanel() override {
			Text::Text("Function name:").show();
			m_nameInput.show();

			NewLine();
			Text::Text("Function signature:").show();
			Text::ColoredText(m_funcSignature->getSigName(), 0xb2d7d9FF).show();
			if (Button::StdButton("Change").present()) {
				delete m_builtinWin;
				const auto panel = new DataTypeSelectorPanel(m_function->getManager()->getProject()->getTypeManager());
				panel->m_controller.m_filter.m_groups.insert(CE::DataType::AbstractType::FunctionSignature);
				panel->handler([&, panel](CE::DataTypePtr dataType)
					{
						m_funcSignature = dynamic_cast<CE::DataType::IFunctionSignature*>(dataType->getType());
						panel->m_window->close();
					});
				m_builtinWin = new PopupBuiltinWindow(panel);
				m_builtinWin->getPos() = GetLeftBottom();
				m_builtinWin->open();
			}
			SameLine();
			if (Button::StdButton("Edit").present()) {
				delete m_editorWin;
				m_editorWin = new StdWindow(CreateDataTypeEditorPanel(m_funcSignature));
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
			Show(m_editorWin);
		}

		void save() {
			m_function->setName(m_nameInput.getInputText());
			m_function->getFunctionSymbol()->setSignature(m_funcSignature);
			m_function->getManager()->getProject()->getTransaction()->markAsDirty(m_function);
		}
	};
};
