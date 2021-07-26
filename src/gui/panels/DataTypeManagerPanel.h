#pragma once
#include "ImageManagerPanel.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Input.h"
#include "controllers/DataTypeManagerController.h"

namespace GUI
{
	class DataTypeListViewGrouping
		: public ListViewGrouping<CE::DataType::IType*>
	{
	public:
		using ListViewGrouping<CE::DataType::IType*>::ListViewGrouping;

	protected:
		bool groupBy(CE::DataType::IType* const& type1, CE::DataType::IType* const& type2) override {
			return type1->getGroup() > type2->getGroup();
		}

		bool renderGroupTop(CE::DataType::IType* const& type, int group_n) override {
			return ImGui::CollapsingHeader(GetGroupName(type).c_str(), ImGuiTreeNodeFlags_DefaultOpen);
		}
	};
	
	class DataTypeManagerPanel : public AbstractPanel
	{
		class DataTypeListView : public DataTypeListViewGrouping
		{
			class DataTypeContextPanel : public AbstractPanel
			{
				DataTypeManagerPanel* m_typeManagerPanel;
				CE::DataType::IType* m_dataType;
				ImVec2 m_winPos;
			public:
				DataTypeContextPanel(CE::DataType::IType* type, DataTypeManagerPanel* typeManagerPanel, ImVec2 winPos)
					: m_dataType(type), m_typeManagerPanel(typeManagerPanel), m_winPos(winPos)
				{}

			private:
				void renderPanel() override {
					if (ImGui::MenuItem("Edit")) {
						// todo: type editor
					}
				}
			};

			DataTypeManagerPanel* m_typeManagerPanel;
		public:
			DataTypeListView(StdListView<CE::DataType::IType*>* listView, DataTypeManagerPanel* typeManagerPanel)
				: DataTypeListViewGrouping(listView), m_typeManagerPanel(typeManagerPanel)
			{}

		private:
			void renderItem(const std::string& text, CE::DataType::IType* const& type, int n) override {
				DataTypeListViewGrouping::renderItem(text, type, n);
				const auto events = GenericEvents(true);
				if (events.isClickedByRightMouseBtn()) {
					delete m_typeManagerPanel->m_typeContextWindow;
					m_typeManagerPanel->m_typeContextWindow = new PopupContextWindow(new DataTypeContextPanel(type, m_typeManagerPanel, GetLeftBottom()));
					m_typeManagerPanel->m_typeContextWindow->open();
				}
			}
		};
		
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
		DataTypeListView* m_listView = nullptr;
		PopupContextWindow* m_typeContextWindow = nullptr;
	public:
		DataTypeManagerPanel(CE::TypeManager* manager)
			: AbstractPanel("DataType manager"), m_manager(manager), m_controller(manager), m_filterControl(this)
		{
			const auto listView = new StdListView(&m_controller.m_listModel);
			m_listView = new DataTypeListView(listView, this);
			m_controller.m_maxItemsCount = 30;
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
			Show(m_typeContextWindow);
		}
	};
};
