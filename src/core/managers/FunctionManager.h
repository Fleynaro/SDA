#pragma once
#include "AbstractManager.h"
#include <Function.h>

namespace DB {
	class FunctionMapper;
};;

namespace CE
{
	class FunctionManager : public AbstractItemManager
	{
	public:
		class Factory : public AbstractFactory
		{
			FunctionManager* m_functionManager;
			DB::FunctionMapper* m_funcMapper;
		public:
			Factory(FunctionManager* functionManager, DB::FunctionMapper* funcMapper, bool markAsNew)
				: m_functionManager(functionManager), m_funcMapper(funcMapper), AbstractFactory(markAsNew)
			{}

			Function* createFunction(Symbol::FunctionSymbol* functionSymbol, ImageDecorator* imageDec, Symbol::StackSymbolTable* stackSymbolTable) const;

			Function* createFunction(Symbol::FunctionSymbol* functionSymbol, ImageDecorator* imageDec) const;

			Function* createFunction(int64_t offset, DataType::IFunctionSignature* funcSignature, ImageDecorator* imageDec, const std::string& name, const std::string& comment = "");
		};

		using Iterator = AbstractIterator<Function>;

		FunctionManager(Project* module);

		~FunctionManager();

		Factory getFactory(bool markAsNew = true);

		void loadFunctions() const;

		Function* findFunctionById(DB::Id id);
	private:
		DB::FunctionMapper* m_funcMapper;
	};
};