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
		Function* m_function = nullptr;
	public:
		FunctionSymbol(SymbolManager* manager, int64_t offset, DataType::IFunctionSignature* funcSignature, const std::string& name, const std::string& comment = "");

		Type getType() override;

		int getSize() override;

		Function* getFunction() const;

		DataType::IFunctionSignature* getSignature() const;

		void setSignature(DataType::IFunctionSignature* funcSignature);

		void setFunction(Function* function);
	};
};