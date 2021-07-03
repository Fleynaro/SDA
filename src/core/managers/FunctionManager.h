#pragma once
#include "AbstractManager.h"
#include <Function.h>
#include <ghidra_sync/DataSyncPacketManagerService.h>

namespace DB {
	class FunctionMapper;
};

namespace CE::Ghidra {
	class FunctionMapper;
};

namespace CE
{
	class FunctionManager : public AbstractItemManager
	{
	public:
		class Factory : public AbstractFactory
		{
			FunctionManager* m_functionManager;
			Ghidra::FunctionMapper* m_ghidraFunctionMapper;
			DB::FunctionMapper* m_funcMapper;
		public:
			Factory(FunctionManager* functionManager, Ghidra::FunctionMapper* ghidraFunctionMapper, DB::FunctionMapper* funcMapper, bool markAsNew)
				: m_functionManager(functionManager), m_ghidraFunctionMapper(ghidraFunctionMapper), m_funcMapper(funcMapper), AbstractFactory(markAsNew)
			{}

			Function* createFunction(Symbol::FunctionSymbol* functionSymbol, ImageDecorator* imageDec, Symbol::SymbolTable* stackSymbolTable) const;

			Function* createFunction(Symbol::FunctionSymbol* functionSymbol, ImageDecorator* imageDec) const;

			Function* createFunction(int64_t offset, DataType::IFunctionSignature* funcSignature, ImageDecorator* imageDec, const std::string& name, const std::string& comment = "");
		};

		using Iterator = AbstractIterator<Function>;
		Ghidra::FunctionMapper* m_ghidraFunctionMapper;

		FunctionManager(Project* module);

		~FunctionManager();

		Factory getFactory(bool markAsNew = true);

		void loadFunctions() const;

		void loadFunctionsFrom(ghidra::packet::SDataFullSyncPacket* dataPacket) const;

		Function* findFunctionById(DB::Id id);

		Function* findFunctionByGhidraId(Ghidra::Id id);
	private:
		DB::FunctionMapper* m_funcMapper;
	};
};