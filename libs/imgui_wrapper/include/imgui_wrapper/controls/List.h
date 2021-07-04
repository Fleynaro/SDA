#pragma once
#include <functional>
#include <list>
#include "Control.h"

namespace GUI
{
	template<typename T>
	class IListModel
	{
	public:
		class Iterator
		{
		public:
			virtual void getNextItem(std::string* text, T* data) = 0;

			virtual bool hasNextItem() = 0;
		};

		using IteratorCallback = std::function<void(Iterator*)>;
		virtual void newIterator(const IteratorCallback& callback) = 0;
	};

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
			typename std::list<ListItem>::iterator m_end;
		public:
			StdIterator(typename std::list<ListItem>::iterator it, typename std::list<ListItem>::iterator end)
				: m_it(it), m_end(end)
			{}

			void getNextItem(std::string* text, T* data) override
			{
				*text = m_it->m_text;
				*data = m_it->m_data;
				++m_it;
			}

			bool hasNextItem() override
			{
				return m_it != m_end;
			}
		};
		
		StdListModel()
		{}

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
			StdIterator iterator(m_listItems.begin(), m_listItems.end());
			callback(&iterator);
		}
	};

	template<typename T>
	class StdListView
		: public Control
	{
	protected:
		IListModel<T>* m_listModel;
		std::function<void(T)> m_clickItemEventHandler;
	
	public:
		StdListView(IListModel<T>* listModel = nullptr)
			: m_listModel(listModel)
		{}

		void present(const std::function<void(T)>& clickItemEventHandler)
		{
			m_clickItemEventHandler = clickItemEventHandler;
			show();
		}
	
	protected:
		void renderControl() override
		{
			m_listModel->newIterator([&](typename IListModel<T>::Iterator* iter)
				{
					while(iter->hasNextItem())
					{
						std::string text;
						T data;
						iter->getNextItem(&text, &data);
						if (ImGui::Selectable(text.c_str())) {
							m_clickItemEventHandler(data);
						}
					}
				});
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
		void renderControl() override
		{
			m_listModel->newIterator([&](typename IListModel<T>::Iterator* iter)
				{
					if (ImGui::BeginTable(getName().c_str(), 2, ImGuiTableFlags_Borders)) {
						while (iter->hasNextItem())
						{
							std::string text;
							T data;
							iter->getNextItem(&text, &data);

							ImGui::TableNextColumn();
							Text::Text("1").show();
							ImGui::TableNextColumn();
							Text::Text(text).show();
						}
						ImGui::EndTable();
					}
				});
		}
	};
};