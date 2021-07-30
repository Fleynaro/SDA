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
	public:
		DataType::IFunctionSignature* m_signature = nullptr;
		int m_paramIdx = -1;
		
		FuncParameterSymbol(SymbolManager* manager = nullptr, DataTypePtr type = nullptr, const std::string& name = "", const std::string& comment = "")
			: AbstractSymbol(manager, type, name, comment)
		{}

		int getParamIdx() const;

		Type getType() override;

		Decompiler::ParameterInfo getParamInfo() const;
		
		Decompiler::Storage getStorage() override;

		FuncParameterSymbol* clone() {
			const auto param = new FuncParameterSymbol(getManager(), getDataType(), getName(), getComment());
			param->setMapper(getMapper());
			return param;
		}
	};
};