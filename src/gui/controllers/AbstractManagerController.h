#pragma once
#include "imgui_wrapper/controls/List.h"
#include "managers/AbstractManager.h"

namespace GUI
{
	template<typename T, typename T2 = CE::AbstractItemManager>
	class AbstractManagerController
	{
	public:
		class ListModel : public IListModel<T*>
		{
		protected:
			AbstractManagerController* m_controller;
		public:
			class Iterator : public IListModel<T*>::Iterator
			{
				typename std::list<T*>::iterator m_it;
			protected:
				ListModel* m_listModel;
			public:
				Iterator(ListModel* listModel)
					: m_listModel(listModel), m_it(listModel->m_controller->m_items.begin())
				{}

				void getNextItem(std::string* text, T** data) override
				{
					*text = getText(*m_it);
					*data = *m_it;
					++m_it;
				}

				bool hasNextItem() override
				{
					return m_it != m_listModel->m_controller->m_items.end();
				}

			protected:
				virtual std::string getText(T* item) = 0;
			};

			ListModel(AbstractManagerController* controller)
				: m_controller(controller)
			{}
		};

		std::list<T*> m_items;
		T2* m_manager;
		int m_maxItemsCount = -1;
		AbstractManagerController(T2* manager)
			: m_manager(manager)
		{}

		bool hasItems() const
		{
			return !m_items.empty();
		}

		virtual bool filter(T* item) = 0;

		virtual void sort()
		{
		}

		void update()
		{
			int itemsCount = 0;
			m_items.clear();
			CE::AbstractItemManager::AbstractIterator<T> iterator(m_manager);
			while (iterator.hasNext())
			{
				auto item = iterator.next();
				if (filter(item)) {
					m_items.push_back(item);
					itemsCount++;
					if (itemsCount == m_maxItemsCount)
						break;
				}
			}
			sort();
		}
	};
};