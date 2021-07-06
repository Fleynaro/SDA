#pragma once
#include "AbstractManagerController.h"
#include "managers/FunctionManager.h"


namespace GUI
{
	class FunctionManagerController : public AbstractManagerController<CE::Function>
	{
	public:
		struct FunctionFilter
		{
			std::string m_name;
		};

		class FunctionListModel : public ListModel
		{
		public:
			using ListModel::ListModel;

		private:
			class FunctionIterator : public Iterator
			{
			public:
				using Iterator::Iterator;

			private:
				std::string getText(CE::Function* item) override
				{
					return item->getName();
				}
			};

			void newIterator(const IteratorCallback& callback) override
			{
				FunctionIterator iterator(this);
				callback(&iterator);
			}
		};

		FunctionFilter m_filter;
		FunctionListModel m_listModel;

		FunctionManagerController(CE::FunctionManager* manager)
			: AbstractManagerController<CE::Function>(manager), m_listModel(this)
		{}

	private:
		bool filter(CE::Function* item) override
		{
			using namespace Helper::String;
			if (!m_filter.m_name.empty() && ToLower(item->getName()).find(ToLower(m_filter.m_name)) == std::string::npos)
				return false;

			return true;
		}
	};
};
