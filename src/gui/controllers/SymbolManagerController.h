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

		void rename(CE::Symbol::ISymbol* symbol, const std::string& name) const {
			symbol->setName(name);
			if (const auto dbSymbol = dynamic_cast<CE::Symbol::AbstractSymbol*>(symbol)) {
				// todo: separate the core logic and the database logic in the way the core don't depend on db
				m_manager->getProject()->getTransaction()->markAsDirty(dbSymbol);
			}
		}

	private:
		bool filter(CE::Symbol::AbstractSymbol* item) override
		{
			using namespace Helper::String;
			if (!m_filter.m_name.empty() && ToLower(item->getName()).find(ToLower(m_filter.m_name)) == std::string::npos)
				return false;

			return true;
		}
	};
};
