#pragma once
#include <functional>
#include <list>
#include <set>

#include "Control.h"
#include "utilities/Helper.h"

namespace GUI
{
	// used for mapping lists to the list model format that can be drawn
	template<typename T>
	class IListModel
	{
	public:
		class Iterator;
		using IteratorCallback = std::function<void(Iterator*)>;

		// iterator interface
		class Iterator
		{
		public:
			virtual void getNextItem(std::string* text, T* data) = 0;

			virtual bool hasNextItem() = 0;
		};

		virtual void newIterator(const IteratorCallback& callback) = 0;
	};

	// standard list model that uses internally std::list
	template<typename T>
	class StdListModel : public IListModel<T>
	{
		struct ListItem
		{
			std::string m_text;
			T m_data;
		};
		std::list<ListItem> m_listItems;
	public:
		class StdIterator : public IListModel<T>::Iterator
		{
			typename std::list<ListItem>::iterator m_it;
			std::list<ListItem>* m_list;
		public:
			StdIterator(std::list<ListItem>* list)
				: m_list(list), m_it(list->begin())
			{}

			void getNextItem(std::string* text, T* data) override
			{
				*text = m_it->m_text;
				*data = m_it->m_data;
				++m_it;
			}

			bool hasNextItem() override
			{
				return m_it != m_list->end();
			}
		};
		
		StdListModel() {}

		void addItem(std::string text, T data)
		{
			ListItem item;
			item.m_text = text;
			item.m_data = data;
			m_listItems.push_back(item);
		}

		void clear()
		{
			m_listItems.clear();
		}

		void newIterator(const IteratorCallback& callback) override
		{
			StdIterator iterator(&m_listItems);
			callback(&iterator);
		}
	};

	// used to draw a list based on a list model
	template<typename T>
	class AbstractListView
		: public Control
	{
	public:
		IListModel<T>* m_listModel;
		using Iterator = typename IListModel<T>::Iterator;
		
		AbstractListView(IListModel<T>* listModel = nullptr)
			: m_listModel(listModel)
		{}
	
		void renderControl() override
		{
			m_listModel->newIterator([&](Iterator* iter)
				{
					renderList(iter);
				});
		}
		
		virtual void renderList(Iterator* iter)
		{
			int n = 0;
			while (iter->hasNextItem())
			{
				std::string text;
				T data;
				iter->getNextItem(&text, &data);
				renderItem(text, data, n++);
			}
		}

		// render a list item
		virtual void renderItem(const std::string& text, const T& data, int n) = 0;
	};

	template<typename T>
	class StdListView
		: public AbstractListView<T>
	{
		EventHandler<T> m_clickItemEventHandler;
	public:
		StdListView(IListModel<T>* listModel = nullptr)
			: AbstractListView<T>(listModel)
		{}

		void handler(const std::function<void(T)>& clickItemEventHandler)
		{
			m_clickItemEventHandler = clickItemEventHandler;
		}
	
	protected:
		void renderItem(const std::string& text, const T& data, int n) override
		{
			if (ImGui::Selectable(text.c_str())) {
				m_clickItemEventHandler(data);
			}
		}
	};

	template<typename T>
	class AbstractTableListView
		: public AbstractListView<T>
	{
	public:
		using AbstractListView<T>::AbstractListView;

		virtual void renderTable(const std::function<void()>& func, int newColsCount) = 0;
				
		virtual void setupColumns() = 0;
	};

	struct ColInfo
	{
		std::string m_name;
		ImGuiTableColumnFlags m_flags;
		float m_width;

		ColInfo(std::string name = "", ImGuiTableColumnFlags flags = ImGuiTableColumnFlags_None, float width = 0.0f)
			: m_name(name), m_flags(flags), m_width(width)
		{}
	};
	
	template<typename T>
	class TableListView
		: public AbstractTableListView<T>,
		public Attribute::Name
	{
	public:
		std::list<ColInfo> m_colsInfo;
		
		TableListView(IListModel<T>* listModel = nullptr, const std::string& name = "", const std::list<ColInfo>& colsInfo = {})
			: AbstractTableListView<T>(listModel), Attribute::Name(name), m_colsInfo(colsInfo)
		{}

	protected:
		void renderTable(const std::function<void()>& content, int newColsCount) override
		{
			if (ImGui::BeginTable(getName().c_str(), static_cast<int>(m_colsInfo.size()) + newColsCount, ImGuiTableFlags_Borders)) {
				content();
				ImGui::EndTable();
			}
		}
		
		void renderList(Iterator* iter) override
		{
			renderTable([&]()
			{
				setupColumns();
				AbstractListView<T>::renderList(iter);
			}, 0);
		}
		
		void renderItem(const std::string& text, const T& data, int n) override
		{
			auto columns = Helper::String::Split(text, ",");
			for(const auto& col : columns)
			{
				ImGui::TableNextColumn();
				Text::Text(col).show();
			}
		}

		void setupColumns() override
		{
			for (const auto& colInfo : m_colsInfo)
				ImGui::TableSetupColumn(colInfo.m_name.c_str(), colInfo.m_flags, colInfo.m_width);
		}
	};

	template<typename T>
	class TableListViewSelector
		: public AbstractTableListView<T>
	{
		AbstractTableListView<T>* m_tableListView;
		EventHandler<T> m_selectEventHandler;
		Button::AbstractButton* m_btn;
	public:
		TableListViewSelector(AbstractTableListView<T>* tableListView, Button::AbstractButton* btn = new Button::ButtonSmall("select"))
			: AbstractTableListView<T>(tableListView->m_listModel), m_tableListView(tableListView), m_btn(btn)
		{}

		~TableListViewSelector() override
		{
			delete m_tableListView;
			delete m_btn;
		}

		void handler(const std::function<void(T)>& selectEventHandler)
		{
			m_selectEventHandler = selectEventHandler;
		}
	
	protected:
		void renderTable(const std::function<void()>& content, int newColsCount) override
		{
			m_tableListView->renderTable(content, newColsCount + 1);
		}
		
		void renderList(Iterator* iter) override
		{
			renderTable([&]()
				{
					setupColumns();
					AbstractTableListView<T>::renderList(iter);
				}, 0);
		}
		
		void renderItem(const std::string& text, const T& data, int n) override
		{
			m_tableListView->renderItem(text, data, n);
			ImGui::TableNextColumn();
			m_btn->setId(this + n);
			if (m_btn->present())
			{
				m_selectEventHandler(data);
			}
		}

		void setupColumns() override
		{
			m_tableListView->setupColumns();
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 50.0f);
		}
	};

	template<typename T>
	class TableListViewMultiSelector
		: public AbstractTableListView<T>
	{
		AbstractTableListView<T>* m_tableListView;
		EventHandler<> m_selectEventHandler;
	public:
		std::set<T> m_selectedItems;
		
		TableListViewMultiSelector(AbstractTableListView<T>* tableListView)
			: AbstractTableListView<T>(tableListView->m_listModel), m_tableListView(tableListView)
		{}

		~TableListViewMultiSelector() override
		{
			delete m_tableListView;
		}

		void handler(const std::function<void()>& selectEventHandler)
		{
			m_selectEventHandler = selectEventHandler;
		}
	
	protected:
		void renderTable(const std::function<void()>& content, int newColsCount) override
		{
			m_tableListView->renderTable(content, newColsCount + 1);
		}
		
		void renderList(Iterator* iter) override
		{
			if (!m_selectedItems.empty()) {
				if (Button::StdButton("Select " + std::to_string(m_selectedItems.size()) + " items.").present())
					m_selectEventHandler();
			}
			renderTable([&]()
			{
				setupColumns();
				AbstractTableListView<T>::renderList(iter);
			}, 0);
		}
		
		void renderItem(const std::string& text, const T& data, int n) override
		{
			m_tableListView->renderItem(text, data, n);
			ImGui::TableNextColumn();
			
			Input::BoolInput checkbox("", m_selectedItems.find(data) != m_selectedItems.end());
			checkbox.setId(this + n);
			if(checkbox.present())
			{
				if (checkbox.isSelected())
					m_selectedItems.insert(data);
				else m_selectedItems.erase(data);
			}
		}

		void setupColumns() override
		{
			m_tableListView->setupColumns();
			ImGui::TableSetupColumn("Select", ImGuiTableColumnFlags_WidthFixed, 50.0f);
		}
	};
};