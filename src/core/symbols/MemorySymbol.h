#pragma once
#include "AbstractSymbol.h"
#include "IMemorySymbol.h"

namespace CE::Symbol
{
	class SymbolTable;

	class AbstractMemorySymbol : public AbstractSymbol, public IMemorySymbol
	{
		int64_t m_offset;
	public:
		AbstractMemorySymbol(SymbolManager* manager, int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment = "");

		int64_t getOffset() const;

		void setOffset(int64_t offset);
	};

	class GlobalVarSymbol : public AbstractMemorySymbol
	{
	public:
		GlobalVarSymbol(SymbolManager* manager, int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment = "");

		Type getType() override;

		Decompiler::Storage getStorage() override;
	};

	class LocalStackVarSymbol : public AbstractMemorySymbol
	{
	public:
		LocalStackVarSymbol(SymbolManager* manager, int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment = "");

		Type getType() override;

		Decompiler::Storage getStorage() override;
	};
};