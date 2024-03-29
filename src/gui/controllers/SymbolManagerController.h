#pragma once
#include "AbstractManagerController.h"
#include "AddressSpace.h"
#include "managers/SymbolManager.h"


namespace GUI
{
	class SymbolManagerController : public AbstractManagerController<CE::Symbol::AbstractSymbol, CE::SymbolManager>
	{
	public:
		struct SymbolFilter
		{
			std::string m_name;
			CE::Symbol::Type m_type = CE::Symbol::GLOBAL_VAR;
		};

		class SymbolListModel : public ListModel
		{
		public:
			SymbolListModel(SymbolManagerController* controller)
				: ListModel(controller)
			{}

		private:
			class SymbolIterator : public Iterator
			{
			public:
				using Iterator::Iterator;

			private:
				std::string getText(CE::Symbol::AbstractSymbol* item) override {
					return item->getName();
				}
			};

			void newIterator(const IteratorCallback& callback) override {
				SymbolIterator iterator(this);
				callback(&iterator);
			}
		};

		SymbolFilter m_filter;
		SymbolListModel m_listModel;

		SymbolManagerController(CE::SymbolManager* manager)
			: AbstractManagerController<CE::Symbol::AbstractSymbol, CE::SymbolManager>(manager), m_listModel(this)
		{}

		CE::SymbolManager::Factory getFactory() const {
			return m_manager->getFactory();
		}

	private:
		bool filter(CE::Symbol::AbstractSymbol* item) override
		{
			using namespace Helper::String;
			if (!m_filter.m_name.empty() && ToLower(item->getName()).find(ToLower(m_filter.m_name)) == std::string::npos)
				return false;
			if (m_filter.m_type != item->getType())
				return false;
			return true;
		}
	};
};
