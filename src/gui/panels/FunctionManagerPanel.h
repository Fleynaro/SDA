#pragma once
#include "ImageManagerPanel.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Input.h"
#include "controllers/FunctionManagerController.h"
#include "controllers/ImageManagerController.h"
#include "editors/FunctionEditorPanel.h"

namespace GUI
{
	class FunctionListViewGrouping
		: public ListViewGrouping<CE::Function*>
	{
	public:
		using ListViewGrouping<CE::Function*>::ListViewGrouping;

	protected:
		bool groupBy(CE::Function* const& func1, CE::Function* const& func2) override {
			return func1->getImage() > func2->getImage();
		}

		bool renderGroupTop(CE::Function* const& func, int group_n) override {
			return ImGui::CollapsingHeader(func->getImage()->getName(), ImGuiTreeNodeFlags_DefaultOpen);
		}
	};
	
	class FunctionManagerPanel : public AbstractPanel
	{
		class FunctionListView : public FunctionListViewGrouping
		{
			class FunctionContextPanel : public AbstractPanel
			{
				FunctionManagerPanel* m_funcManagerPanel;
				CE::Function* m_function;
				ImVec2 m_winPos;
			public:
				FunctionContextPanel(CE::Function* function, FunctionManagerPanel* funcManagerPanel, ImVec2 winPos)
					: m_function(function), m_funcManagerPanel(funcManagerPanel), m_winPos(winPos)
				{}

			private:
				void renderPanel() override {
					if (ImGui::MenuItem("Edit")) {
						delete m_funcManagerPanel->m_functionEditor;
						m_funcManagerPanel->m_functionEditor = new StdWindow(new FunctionEditorPanel(m_function));
					}
				}
			};
			
			FunctionManagerPanel* m_funcManagerPanel;
		public:
			FunctionListView(StdListView<CE::Function*>* listView, FunctionManagerPanel* funcManagerPanel)
				: FunctionListViewGrouping(listView), m_funcManagerPanel(funcManagerPanel)
			{}
		
		private:
			void renderItem(const std::string& text, CE::Function* const& func, int n) override {
				FunctionListViewGrouping::renderItem(text, func, n);
				const auto events = GenericEvents(true);
				if (events.isClickedByLeftMouseBtn()) {
					if (m_funcManagerPanel->m_selectFuncEventHandler.isInit())
						m_funcManagerPanel->m_selectFuncEventHandler(func);
				}
				if (events.isClickedByRightMouseBtn()) {
					delete m_funcManagerPanel->m_funcContextWindow;
					m_funcManagerPanel->m_funcContextWindow = new PopupContextWindow(new FunctionContextPanel(func, m_funcManagerPanel, GetLeftBottom()));
					m_funcManagerPanel->m_funcContextWindow->open();
				}
			}
		};
		
		class FunctionFilter : public Control
		{
			FunctionManagerPanel* m_panel;
			Input::TextInput m_search_input;
		public:
			ImageSelector m_imageSelector;
			bool m_imageSelectorEnabled = false;
			bool m_isUpdated = false;
			
			FunctionFilter(FunctionManagerPanel* panel)
				: m_panel(panel), m_imageSelector(panel->m_manager->getProject()->getImageManager())
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

				if (m_imageSelectorEnabled) {
					m_imageSelector.show();
					if (m_imageSelector.m_isUpdated) {
						filter.m_images = *m_imageSelector.m_selectedItems;
						m_isUpdated = true;
					}
				}
			}
		};

		CE::FunctionManager* m_manager;
		FunctionManagerController m_controller;
		FunctionFilter m_filterControl;
		FunctionListView* m_listView = nullptr;
		PopupContextWindow* m_funcContextWindow = nullptr;
		StdWindow* m_functionEditor = nullptr;
		EventHandler<CE::Function*> m_selectFuncEventHandler;
	public:
		FunctionManagerPanel(CE::FunctionManager* manager)
			: AbstractPanel("Function manager"), m_manager(manager), m_controller(manager), m_filterControl(this)
		{
			/*m_listView = new TableListView(&m_controller.m_listModel, {
				ColInfo("Function")
			});*/
			const auto listView = new StdListView(&m_controller.m_listModel);
			m_listView = new FunctionListView(listView, this);
			m_controller.m_maxItemsCount = 50;
			m_controller.update();
		}

		~FunctionManagerPanel() override {
			delete m_listView;
		}

		void selectFuncEventHandler(const std::function<void(CE::Function*)>& eventHandler) {
			m_selectFuncEventHandler = eventHandler;
		}

	protected:
		void renderPanel() override {
			m_filterControl.show();
			if(m_filterControl.m_isUpdated)
			{
				m_controller.update();
			}
			
			m_listView->show();
			Show(m_functionEditor);
			Show(m_funcContextWindow);
		}
	};
};
