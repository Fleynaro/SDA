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

			Symbol::GlobalSymbolTable* createGlobalSymbolTable() const;

			Symbol::StackSymbolTable* createStackSymbolTable() const;

		private:
			void initSymbolTable(Symbol::AbstractSymbolTable* symbolTable) const;
		};

		using Iterator = AbstractIterator<Symbol::AbstractSymbolTable>;

		SymbolTableManager(Project* module);

		void loadSymTables() const;

		Factory getFactory(bool markAsNew = true);

		Symbol::AbstractSymbolTable* findSymbolTableById(DB::Id id);

	private:
		DB::SymbolTableMapper* m_symbolTableMapper;
	};
};