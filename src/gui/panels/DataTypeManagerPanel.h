#pragma once
#include "ImageManagerPanel.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Input.h"
#include "controllers/DataTypeManagerController.h"
#include "editors/DataTypeEditorPanel.h"

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

	class DataTypeSelectorPanel : public AbstractPanel
	{
		Input::TextInput m_input;
		DataTypeListViewGrouping* m_groupingListView;
		EventHandler<CE::DataTypePtr> m_eventHandler;
	public:
		DataTypeManagerController m_controller;
		std::string m_errorMessage;
		
		DataTypeSelectorPanel(CE::TypeManager* manager, const std::string& name = "")
			: m_controller(manager)
		{
			m_input.setInputText(name);
			m_input.focus();
			m_controller.m_maxItemsCount = 10;

			const auto listView = new StdListView(&m_controller.m_listModel);
			listView->handler([&](CE::DataType::IType* type)
				{
					m_input.setInputText(type->getName());
				});
			m_groupingListView = new DataTypeListViewGrouping(listView);
		}

		~DataTypeSelectorPanel() {
			delete m_groupingListView;
		}

		void handler(const std::function<void(CE::DataTypePtr)>& eventHandler) {
			m_eventHandler = eventHandler;
		}
	
	private:
		void renderPanel() override {
			m_input.show();
			SameLine();
			if (Button::StdButton("Ok").present()) {
				// todo: check new size that can overlap other symbols
				const auto dataType = m_controller.parseDataType(m_input.getInputText());
				if(m_eventHandler.isInit())
					m_eventHandler(dataType);
			}
			if (!m_errorMessage.empty()) {
				Text::Text("Error: " + m_errorMessage).show();
			}
			if (m_controller.hasItems())
				m_groupingListView->show();

			if (m_input.isTextEntering()) {
				m_controller.m_filter.m_name = m_input.getInputText();
				m_controller.update();
			}
		}
	};
	
	class DataTypeManagerPanel : public AbstractPanel
	{
		class DataTypeListView : public DataTypeListViewGrouping
		{
			class DataTypeContextPanel : public AbstractPanel
			{
				DataTypeManagerPanel* m_typeManagerPanel;
				CE::DataType::IUserDefinedType* m_dataType;
				ImVec2 m_winPos;
			public:
				DataTypeContextPanel(CE::DataType::IUserDefinedType* dataType, DataTypeManagerPanel* typeManagerPanel, ImVec2 winPos)
					: m_dataType(dataType), m_typeManagerPanel(typeManagerPanel), m_winPos(winPos)
				{}

			private:
				void renderPanel() override {
					if (ImGui::MenuItem("Edit")) {
						delete m_typeManagerPanel->m_dataTypeEditor;
						m_typeManagerPanel->m_dataTypeEditor = new StdWindow(CreateDataTypeEditorPanel(m_dataType));
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
					if (const auto userDefinedType = dynamic_cast<CE::DataType::IUserDefinedType*>(type)) {
						delete m_typeManagerPanel->m_typeContextWindow;
						m_typeManagerPanel->m_typeContextWindow = new PopupContextWindow(new DataTypeContextPanel(userDefinedType, m_typeManagerPanel, GetLeftBottom()));
						m_typeManagerPanel->m_typeContextWindow->open();
					}
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
		StdWindow* m_dataTypeEditor = nullptr;
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
			Show(m_dataTypeEditor);
		}
	};
};
