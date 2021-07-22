#pragma once
#include "ImageManagerPanel.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Input.h"
#include "controllers/DataTypeManagerController.h"
#include "controllers/ImageManagerController.h"

namespace GUI
{
	class DataTypeManagerPanel : public AbstractPanel
	{
	public:
		class DataTypeFilter : public Control
		{
			DataTypeManagerPanel* m_panel;
			Input::TextInput m_search_input;
		public:
			bool m_isUpdated = false;
			
			DataTypeFilter(DataTypeManagerPanel* panel)
				: m_panel(panel)
			{}
		protected:
			void renderControl() override {
				auto& filter = m_panel->m_controller.m_filter;
				
				m_isUpdated = false;
				if (m_search_input.isTextEntering()) {
					filter.m_name = m_search_input.getInputText();
					m_isUpdated = true;
				}

				m_search_input.show();
			}
		};

		CE::TypeManager* m_manager;
		DataTypeManagerController m_controller;
		DataTypeFilter m_filterControl;
		AbstractTableListView<CE::DataType::IType*>* m_listView = nullptr;
		DataTypeManagerPanel(CE::TypeManager* manager)
			: AbstractPanel("DataType manager"), m_manager(manager), m_controller(manager), m_filterControl(this)
		{
			m_listView = new TableListView(&m_controller.m_listModel, {
				ColInfo("DataType")
			});
			m_controller.update();
		}

		~DataTypeManagerPanel() override
		{
			delete m_listView;
		}

	protected:
		void renderPanel() override {
			m_filterControl.show();
			if(m_filterControl.m_isUpdated)
			{
				m_controller.update();
			}
			
			m_listView->show();
		}
	};
};
