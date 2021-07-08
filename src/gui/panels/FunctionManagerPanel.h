#pragma once
#include "ImageViewerPanel.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Input.h"
#include "controllers/FunctionManagerController.h"
#include "controllers/ImageManagerController.h"

namespace GUI
{
	class FunctionManagerPanel : public AbstractPanel
	{
	public:
		class FunctionFilter : public Control
		{
			FunctionManagerPanel* m_panel;
			Input::TextInput m_search_input;
		public:
			ImageSelector m_imageSelector;
			bool m_isFilterUpdated = false;
			
			FunctionFilter(FunctionManagerPanel* panel)
				: m_panel(panel), m_imageSelector(panel->m_manager->getProject()->getImageManager())
			{}

		protected:
			void renderControl() override {
				auto& filter = m_panel->m_controller.m_filter;
				
				m_isFilterUpdated = false;
				if (m_search_input.isTextEntering()) {
					filter.m_name = m_search_input.getInputText();
					m_isFilterUpdated = true;
				}

				m_search_input.show();
				m_imageSelector.show();
				filter.m_images = *m_imageSelector.m_selectedImages;
			}
		};

		CE::FunctionManager* m_manager;
		FunctionManagerController m_controller;
		FunctionFilter m_filterControl;
		AbstractTableListView<CE::Function*>* m_listView = nullptr;
		FunctionManagerPanel(CE::FunctionManager* manager)
			: AbstractPanel("Function manager"), m_manager(manager), m_controller(manager), m_filterControl(this)
		{
			m_listView = new TableListView(&m_controller.m_listModel, "table", {
				ColInfo("Function")
			});
		}

		~FunctionManagerPanel() override
		{
			delete m_listView;
		}

	protected:
		void renderPanel() override {
			m_filterControl.show();
			if(m_filterControl.m_isFilterUpdated)
			{
				m_controller.update();
			}
			
			m_listView->show();
		}
	};
};
