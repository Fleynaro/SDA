#pragma once
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Button.h"
#include "imgui_wrapper/controls/Input.h"

namespace GUI
{
	class BuiltinTextInputPanel : public AbstractPanel
	{
		Input::TextInput m_input;
		EventHandler<const std::string&> m_eventHandler;
	public:
		BuiltinTextInputPanel(const std::string& name = "")
		{
			m_input.setInputText(name);
			m_input.focus();
		}

		void handler(const std::function<void(const std::string&)>& eventHandler) {
			m_eventHandler = eventHandler;
		}

	private:
		void renderPanel() override {
			m_input.show();
			SameLine();
			if (Button::StdButton("Ok").present()) {
				m_window->close();
				if (m_eventHandler.isInit())
					m_eventHandler(m_input.getInputText());
			}
		}
	};

	class BuiltinIntegerInputPanel : public AbstractPanel
	{
		Input::IntegerInput m_input;
		EventHandler<const int&> m_eventHandler;
	public:
		BuiltinIntegerInputPanel(const int& value = 0)
		{
			m_input.setInputValue(value);
			m_input.focus();
		}

		void handler(const std::function<void(const int&)>& eventHandler) {
			m_eventHandler = eventHandler;
		}

	private:
		void renderPanel() override {
			m_input.show();
			SameLine();
			if (Button::StdButton("Ok").present()) {
				m_window->close();
				if (m_eventHandler.isInit())
					m_eventHandler(m_input.getInputValue());
			}
		}
	};
};