#pragma once
#include "AbstractManager.h"
#include <symbols/Symbols.h>
#include <managers/TypeManager.h>

namespace DB {
	class SymbolMapper;
};

namespace CE
{
	class SymbolManager : public AbstractItemManager
	{
	public:
		class Factory : public AbstractFactory
		{
			SymbolManager* m_symbolManager;
			DB::SymbolMapper* m_symbolMapper;
		public:
			Factory(SymbolManager* symbolManager, DB::SymbolMapper* symbolMapper, bool markAsNew)
				: m_symbolManager(symbolManager), m_symbolMapper(symbolMapper), AbstractFactory(markAsNew)
			{}

			Symbol::FuncParameterSymbol* createFuncParameterSymbol(DataTypePtr type, const std::string& name, const std::string& comment = "") const;

			Symbol::StructFieldSymbol* createStructFieldSymbol(int bitSize, DataTypePtr type, const std::string& name, const std::string& comment = "") const;

			Symbol::FunctionSymbol* createFunctionSymbol(int64_t offset, DataType::IFunctionSignature* funcSignature, const std::string& name, const std::string& comment = "") const;

			Symbol::LocalInstrVarSymbol* createLocalInstrVarSymbol(DataTypePtr type, const std::string& name, const std::string& comment = "") const;

			Symbol::GlobalVarSymbol* createGlobalVarSymbol(int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment = "") const;

			Symbol::LocalStackVarSymbol* createLocalStackVarSymbol(int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment = "") const;
		private:
			void bind(Symbol::AbstractSymbol* symbol) const;
		};

		using Iterator = AbstractIterator<Symbol::AbstractSymbol>;
		
		SymbolManager(Project* module);

		Factory getFactory(bool markAsNew = true);

		void loadSymbols() const;

		Symbol::AbstractSymbol* findSymbolById(DB::Id id);

		Symbol::GlobalVarSymbol* getDefGlobalVarSymbol();

	private:
		DB::SymbolMapper* m_symbolMapper;
		Symbol::GlobalVarSymbol* m_defGlobVarSymbol;
	};
};