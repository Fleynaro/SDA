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

		Type getType() override {
			return FUNCTION;
		}

		int getSize() override {
			return 1;
		}

		Function* getFunction() {
			return m_function;
		}

		DataType::IFunctionSignature* getSignature() {
			return m_funcSignature;
		}

		void setFunction(Function* function) {
			m_function = function;
		}
	};
};