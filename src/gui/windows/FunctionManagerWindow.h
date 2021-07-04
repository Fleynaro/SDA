#pragma once
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/Input.h"
#include "controllers/FunctionManagerController.h"


namespace GUI
{
	class FunctionManagerWindow : public Window
	{
		FunctionManagerController m_controller;
		Input::TextInput m_search_input;
		TableListView<CE::Function*> m_tableListView;
	
	public:
		FunctionManagerWindow(CE::FunctionManager* functionManager)
			: Window("Function manager"), m_controller(functionManager)
		{
			m_tableListView = TableListView(&m_controller.m_model, "listView");
		}

	protected:
		void renderWindow() override {
			if (!m_search_input.getInputText().empty()) {
				m_controller.m_filter.m_name = m_search_input.getInputText();
				m_controller.update();
			}

			m_search_input.show();
			m_tableListView.present([&](CE::Function* function)
				{
					
				});
		}
	};
};
