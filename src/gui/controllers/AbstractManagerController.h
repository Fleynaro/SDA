#pragma once
#include "imgui_wrapper/controls/List.h"

namespace GUI
{
	template<typename T>
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
				AbstractManagerController* m_controller;
			public:
				Iterator(AbstractManagerController* controller)
					: m_controller(controller), m_it(controller->m_items.begin())
				{}

				void getNextItem(std::string* text, T** data) override
				{
					*text = getText(*m_it);
					*data = *m_it;
					++m_it;
				}

				bool hasNextItem() override
				{
					return m_it != m_controller->m_items.end();
				}

			protected:
				virtual std::string getText(T* item) = 0;
			};

			ListModel(AbstractManagerController* controller)
				: m_controller(controller)
			{}
		};

		std::list<T*> m_items;
		CE::AbstractItemManager* m_manager;
		AbstractManagerController(CE::AbstractItemManager* manager)
			: m_manager(manager)
		{}

		virtual bool filter(T* item) = 0;

		void update()
		{
			m_items.clear();
			CE::AbstractItemManager::AbstractIterator<T> iterator(m_manager);
			while (iterator.hasNext())
			{
				auto item = iterator.next();
				if (filter(item)) {
					m_items.push_back(item);
				}
			}
		}
	};
};