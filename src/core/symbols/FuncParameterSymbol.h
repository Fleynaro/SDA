#pragma once
#include "AbstractSymbol.h"
#include "IMemorySymbol.h"

namespace CE::DataType {
	class IFunctionSignature;
};

namespace CE::Symbol
{
	class FuncParameterSymbol : public AbstractSymbol, public IMemorySymbol
	{
		int m_paramIdx;
		DataType::IFunctionSignature* m_signature;
	public:
		FuncParameterSymbol(SymbolManager* manager, int paramIdx, DataType::IFunctionSignature* signature, DataTypePtr type, const std::string& name, const std::string& comment = "")
			: m_paramIdx(paramIdx), m_signature(signature), AbstractSymbol(manager, type, name, comment)
		{}

		int getParamIdx();

		Type getType() override;

		Decompiler::Storage getStorage() override;

		void setFuncSignature(DataType::IFunctionSignature* signature);
	};
};