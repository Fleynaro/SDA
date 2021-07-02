#pragma once
#include "AbstractSymbol.h"

namespace CE::Symbol
{
	class LocalInstrVarSymbol : public AbstractSymbol
	{
	public:
		std::list<int64_t> m_instrOffsets;

		LocalInstrVarSymbol(SymbolManager* manager, DataTypePtr type, const std::string& name, const std::string& comment = "");

		Type getType() override;
	};
};