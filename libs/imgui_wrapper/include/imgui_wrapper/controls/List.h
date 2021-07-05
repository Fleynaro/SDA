#pragma once
#include <functional>
#include <list>
#include "Control.h"

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
			while (iter->hasNextItem())
			{
				std::string text;
				T data;
				iter->getNextItem(&text, &data);
				renderItem(text, data);
			}
		}

		// render a list item
		virtual void renderItem(const std::string& text, const T& data) = 0;
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
		void renderItem(const std::string& text, const T& data) override
		{
			if (ImGui::Selectable(text.c_str())) {
				m_clickItemEventHandler(data);
			}
		}
	};

	template<typename T>
	class TableListView
		: public StdListView<T>,
		public Attribute::Name
	{
	public:
		TableListView(IListModel<T>* listModel = nullptr, const std::string& name = "")
			: StdListView<T>(listModel), Attribute::Name(name)
		{}

	protected:
		void renderList(Iterator* iter) override
		{
			if (ImGui::BeginTable(getName().c_str(), 2, ImGuiTableFlags_Borders)) {
				AbstractListView<T>::renderList(iter);
				ImGui::EndTable();
			}
		}
		
		void renderItem(const std::string& text, const T& data) override
		{
			ImGui::TableNextColumn();
			Text::Text("1").show();
			ImGui::TableNextColumn();
			Text::Text(text).show();
		}
	};
};