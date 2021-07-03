#pragma once
#include "AbstractManager.h"
#include <SymbolTable.h>

namespace DB {
	class SymbolTableMapper;
};

namespace CE
{
	class SymbolTableManager : public AbstractItemManager
	{
	public:
		class Factory : public AbstractFactory
		{
			SymbolTableManager* m_symbolTableManager;
			DB::SymbolTableMapper* m_symbolTableMapper;
		public:
			Factory(SymbolTableManager* symbolTableManager, DB::SymbolTableMapper* symbolTableMapper, bool markAsNew)
				: m_symbolTableManager(symbolTableManager), m_symbolTableMapper(symbolTableMapper), AbstractFactory(markAsNew)
			{}

			Symbol::SymbolTable* createSymbolTable(Symbol::SymbolTable::SymbolTableType type) const;
		};

		using Iterator = AbstractIterator<Symbol::SymbolTable>;

		SymbolTableManager(Project* module);

		void loadSymTables() const;

		Factory getFactory(bool markAsNew = true);

		Symbol::SymbolTable* findSymbolTableById(DB::Id id);

	private:
		DB::SymbolTableMapper* m_symbolTableMapper;
	};
};