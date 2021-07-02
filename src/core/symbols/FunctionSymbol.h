#pragma once
#include "MemorySymbol.h"

namespace CE {
	class Function;

	namespace DataType {
		class IFunctionSignature;
	};
};

namespace CE::Symbol
{
	class FunctionSymbol : public GlobalVarSymbol
	{
		DataType::IFunctionSignature* m_funcSignature;
		Function* m_function;
	public:
		FunctionSymbol(SymbolManager* manager, int64_t offset, DataType::IFunctionSignature* funcSignature, const std::string& name, const std::string& comment = "");

		Type getType() override;

		int getSize() override;

		Function* getFunction();

		DataType::IFunctionSignature* getSignature();

		void setFunction(Function* function);
	};
};