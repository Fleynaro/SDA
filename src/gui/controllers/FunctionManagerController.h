#pragma once
#include "imgui_wrapper/controls/List.h"
#include "managers/FunctionManager.h"


namespace GUI
{
	class FunctionManagerController
	{
	public:
		using ListModel = StdListModel<CE::Function*>;
		struct Filter
		{
			std::string m_name;
		};

		CE::FunctionManager* m_functionManager;
		Filter m_filter;
		ListModel m_model;
	
		FunctionManagerController(CE::FunctionManager* functionManager)
			: m_functionManager(functionManager)
		{}

		void applyFilter(Filter filter)
		{
			m_filter = filter;
			update();
		}

		void update()
		{
			m_model.clear();
			
			CE::FunctionManager::Iterator iterator(m_functionManager);
			while(iterator.hasNext())
			{
				auto function = iterator.next();
				if (function->getName().find(m_filter.m_name) != std::string::npos) {
					m_model.addItem(function->getName(), function);
				}
			}
		}
	};
};
