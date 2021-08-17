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
		std::string m_errorMessage;
		
		BuiltinTextInputPanel(const std::string& name = "")
		{
			m_input.setInputText(name);
			m_input.setFlags(ImGuiInputTextFlags_EnterReturnsTrue);
			m_input.focus();
		}

		void handler(const std::function<void(const std::string&)>& eventHandler) {
			m_eventHandler = eventHandler;
		}

	private:
		void renderPanel() override {
			m_input.show();
			SameLine();
			if (Button::StdButton("Ok").present() || m_input.isTextEntering()) {
				if (m_eventHandler.isInit())
					m_eventHandler(m_input.getInputText());
			}
			if(!m_errorMessage.empty()) {
				Text::Text("Error: " + m_errorMessage).show();
			}
		}
	};

	class BuiltinIntegerInputPanel : public AbstractPanel
	{
		Input::IntegerInput m_input;
		EventHandler<const int&> m_eventHandler;
	public:
		std::string m_errorMessage;
		
		BuiltinIntegerInputPanel(const int& value = 0)
		{
			m_input.setInputValue(value);
			m_input.setFlags(ImGuiInputTextFlags_EnterReturnsTrue);
			m_input.focus();
		}

		void handler(const std::function<void(const int&)>& eventHandler) {
			m_eventHandler = eventHandler;
		}

	private:
		void renderPanel() override {
			m_input.show();
			SameLine();
			if (Button::StdButton("Ok").present() || m_input.isValueEntering()) {
				if (m_eventHandler.isInit())
					m_eventHandler(m_input.getInputValue());
			}
			if (!m_errorMessage.empty()) {
				Text::Text("Error: " + m_errorMessage).show();
			}
		}
	};
};