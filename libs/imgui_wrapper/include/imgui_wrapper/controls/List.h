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
		IListModel<T>* m_listModel;
	public:
		AbstractListView(IListModel<T>* listModel = nullptr)
			: m_listModel(listModel)
		{}
	
	private:
		void renderControl() override
		{
			m_listModel->newIterator([&](typename IListModel<T>::Iterator* iter)
				{
					renderList(iter);
				});
		}

	protected:
		using Iterator = typename IListModel<T>::Iterator;
		
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
		std::function<void(T)> m_clickItemEventHandler;
	public:
		StdListView(IListModel<T>* listModel = nullptr)
			: AbstractListView<T>(listModel)
		{}

		void present(const std::function<void(T)>& clickItemEventHandler)
		{
			m_clickItemEventHandler = clickItemEventHandler;
			show();
		}
	
	protected:
		void renderItem(const std::string& text, const T& data, int n) override
		{
			if (ImGui::Selectable(text.c_str())) {
				m_clickItemEventHandler(data);
			}
		}
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
		: public AbstractListView<T>,
		public Attribute::Name
	{
	public:
		std::list<ColInfo> m_colsInfo;
		
		TableListView(IListModel<T>* listModel = nullptr, const std::string& name = "", const std::list<ColInfo>& colsInfo = {})
			: AbstractListView<T>(listModel), Attribute::Name(name), m_colsInfo(colsInfo)
		{}

	protected:
		void renderList(Iterator* iter) override
		{
			if (ImGui::BeginTable(getName().c_str(), std::max(1, (int)m_colsInfo.size()), ImGuiTableFlags_Borders)) {
				for(const auto& colInfo : m_colsInfo)
					ImGui::TableSetupColumn(colInfo.m_name.c_str(), colInfo.m_flags, colInfo.m_width);
				AbstractListView<T>::renderList(iter);
				ImGui::EndTable();
			}
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
	};

	template<typename T>
	class TableListViewSelector
		: public TableListView<T>
	{
		std::function<void(T)> m_clickItemEventHandler;
	public:
		using TableListView<T>::TableListView;

		void present(const std::function<void(T)>& clickItemEventHandler)
		{
			m_clickItemEventHandler = clickItemEventHandler;
			show();
		}
	
	protected:
		void renderItem(const std::string& text, const T& data, int n) override
		{
			TableListView<T>::renderItem(text, data, n);
			ImGui::TableNextColumn();
			auto btn = Button::ButtonSmall("select");
			btn.setId(this + n);
			if (btn.present())
			{
				m_clickItemEventHandler(data);
			}
		}
	};

	template<typename T>
	class TableListViewMultiSelector
		: public TableListViewSelector<T>
	{
	public:
		std::set<T> m_selectedItems;
		
		using TableListViewSelector<T>::TableListViewSelector;

	protected:
		void renderList(Iterator* iter) override
		{
			if(auto selItemsCount = m_selectedItems.size())
				Text::Text("Selected " + std::to_string(selItemsCount) + " items.").show();
			TableListViewSelector<T>::renderList(iter);
		}
		
		void renderItem(const std::string& text, const T& data, int n) override
		{
			TableListView<T>::renderItem(text, data, n);
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
	};
};