#pragma once
#include "AbstractSymbol.h"
#include "decompiler/DecStorage.h"

namespace CE::DataType {
	class IFunctionSignature;
};

namespace CE::Symbol
{
	class FuncParameterSymbol : public AbstractSymbol
	{
	public:
		DataType::IFunctionSignature* m_signature = nullptr;
		int m_paramIdx = -1;
		
		FuncParameterSymbol(SymbolManager* manager = nullptr, DataTypePtr type = nullptr, const std::string& name = "", const std::string& comment = "")
			: AbstractSymbol(manager, type, name, comment)
		{}

		Type getType() override;

		Decompiler::Storage getStorage() override;

		int getParamIdx() const;

		Decompiler::ParameterInfo getParamInfo() const;

		FuncParameterSymbol* clone() {
			const auto param = new FuncParameterSymbol(getManager(), getDataType(), getName(), getComment());
			param->setMapper(getMapper());
			return param;
		}
	};
};