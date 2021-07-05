#pragma once
#include "imgui_wrapper/controls/List.h"
#include "managers/FunctionManager.h"


namespace GUI
{
	class FunctionManagerController
	{
	public:
		struct Filter
		{
			std::string m_name;
		};

		class ListModel : public IListModel<CE::Function*>
		{
			FunctionManagerController* m_controller;
		public:
			class Iterator : public IListModel<CE::Function*>::Iterator
			{
				typename std::list<CE::Function*>::iterator m_it;
				FunctionManagerController* m_controller;
			public:
				Iterator(FunctionManagerController* controller)
					: m_controller(controller), m_it(controller->m_items.begin())
				{}

				void getNextItem(std::string* text, CE::Function** data) override
				{
					*text = getName(*m_it);
					*data = *m_it;
					++m_it;
				}

				bool hasNextItem() override
				{
					return m_it != m_controller->m_items.end();
				}

			protected:
				virtual std::string getName(CE::Function* item)
				{
					return item->getName();
				}
			};

			ListModel(FunctionManagerController* controller)
				: m_controller(controller)
			{}

			void newIterator(const IteratorCallback& callback) override
			{
				Iterator iterator(m_controller);
				callback(&iterator);
			}
		};

		CE::FunctionManager* m_manager;
		Filter m_filter;
		std::list<CE::Function*> m_items;
		ListModel m_listModel;
	
		FunctionManagerController(CE::FunctionManager* functionManager)
			: m_manager(functionManager), m_listModel(this)
		{}

		void update()
		{
			m_items.clear();
			CE::FunctionManager::Iterator iterator(m_manager);
			while(iterator.hasNext())
			{
				auto function = iterator.next();
				if (function->getName().find(m_filter.m_name) != std::string::npos) {
					m_items.push_back(function);
				}
			}
		}
	};
};
