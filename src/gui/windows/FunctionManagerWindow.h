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
		TableListViewMultiSelector<CE::Function*> m_tableListView;
	
	public:
		FunctionManagerWindow(CE::FunctionManager* functionManager)
			: Window("Function manager"), m_controller(functionManager)
		{
			m_tableListView = TableListViewMultiSelector(&m_controller.m_listModel, "listView", {
				ColInfo("Function"),
				ColInfo("Select", ImGuiTableColumnFlags_WidthFixed, 50.0f)
			});
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
