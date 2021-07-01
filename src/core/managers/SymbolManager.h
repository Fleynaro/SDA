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

			Symbol::FuncParameterSymbol* createFuncParameterSymbol(int paramIdx, DataType::IFunctionSignature* signature, DataTypePtr type, const std::string& name, const std::string& comment = "");

			Symbol::StructFieldSymbol* createStructFieldSymbol(int absBitOffset, int bitSize, DataType::IStructure* structure, DataTypePtr type, const std::string& name, const std::string& comment = "");

			Symbol::FunctionSymbol* createFunctionSymbol(int64_t offset, DataType::IFunctionSignature* funcSignature, const std::string& name, const std::string& comment = "");

			Symbol::LocalInstrVarSymbol* createLocalInstrVarSymbol(DataTypePtr type, const std::string& name, const std::string& comment = "");

			Symbol::GlobalVarSymbol* createGlobalVarSymbol(int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment = "");

			Symbol::LocalStackVarSymbol* createLocalStackVarSymbol(int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment = "");
		private:
			void bind(Symbol::AbstractSymbol* symbol);
		};

		using Iterator = AbstractIterator<Symbol::AbstractSymbol>;
		
		SymbolManager(Project* module);

		Factory getFactory(bool markAsNew = true);

		void loadSymbols();

		Symbol::AbstractSymbol* findSymbolById(DB::Id id);

	private:
		DB::SymbolMapper* m_symbolMapper;
	};
};