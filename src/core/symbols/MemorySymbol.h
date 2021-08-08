#pragma once
#include "AbstractSymbol.h"
#include "IMemorySymbol.h"

namespace CE::Symbol
{
	class GlobalSymbolTable;
	class StackSymbolTable;

	class AbstractMemorySymbol : public AbstractSymbol
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
		GlobalSymbolTable* m_globalSymbolTable = nullptr;
		
		GlobalVarSymbol(SymbolManager* manager, int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment = "");

		Type getType() override;
	};

	class LocalStackVarSymbol : public AbstractMemorySymbol
	{
	public:
		StackSymbolTable* m_stackSymbolTable = nullptr;
		
		LocalStackVarSymbol(SymbolManager* manager, int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment = "");

		Type getType() override;
	};
};