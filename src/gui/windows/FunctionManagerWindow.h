#pragma once
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/Input.h"
#include "controllers/FunctionManagerController.h"
#include "controllers/AddressSpaceManagerController.h"


namespace GUI
{
	class FunctionManagerWindow : public Window
	{
	public:
		class FunctionFilter : public Control
		{
			FunctionManagerController::FunctionFilter* m_filter;
			Input::TextInput m_search_input;
		public:
			bool m_isFilterUpdated = false;
			
			FunctionFilter(FunctionManagerController::FunctionFilter* filter)
				: m_filter(filter)
			{}

		protected:
			void renderControl() override {
				m_isFilterUpdated = false;
				if (m_search_input.isTextEntering()) {
					m_filter->m_name = m_search_input.getInputText();
					m_isFilterUpdated = true;
				}

				m_search_input.show();
			}
		};

		FunctionManagerController m_controller;
		FunctionFilter m_filterControl;
		AbstractTableListView<CE::Function*>* m_listView = nullptr;
		FunctionManagerWindow(CE::FunctionManager* manager)
			: Window("Function manager"), m_controller(manager), m_filterControl(&m_controller.m_filter)
		{
			m_listView = new TableListView(&m_controller.m_listModel, "table", {
				ColInfo("Function")
			});
		}

		~FunctionManagerWindow() override
		{
			delete m_listView;
		}

	protected:
		void renderWindow() override {
			m_filterControl.show();
			if(m_filterControl.m_isFilterUpdated)
			{
				m_controller.update();
			}
			
			m_listView->show();
		}
	};
};
