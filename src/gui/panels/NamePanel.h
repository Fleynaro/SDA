#pragma once
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Button.h"
#include "imgui_wrapper/controls/Input.h"

namespace GUI
{
	class NamePanel : public AbstractPanel
	{
		Input::TextInput m_input;
		EventHandler<const std::string&> m_eventHandler;
	public:
		NamePanel(const std::string& name = "")
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
};