#include "MemorySymbol.h"

using namespace CE;
using namespace Symbol;

LocalStackVarSymbol::LocalStackVarSymbol(SymbolManager* manager, int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment)
	: AbstractMemorySymbol(manager, offset, type, name, comment)
{}

Type LocalStackVarSymbol::getType() {
	return LOCAL_STACK_VAR;
}

AbstractMemorySymbol::AbstractMemorySymbol(SymbolManager* manager, int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment)
	: AbstractSymbol(manager, type, name, comment), m_offset(offset)
{}

int64_t AbstractMemorySymbol::getOffset() const
{
	return m_offset;
}

void AbstractMemorySymbol::setOffset(int64_t offset) {
	m_offset = offset;
}

GlobalVarSymbol::GlobalVarSymbol(SymbolManager* manager, int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment)
	: AbstractMemorySymbol(manager, offset, type, name, comment)
{}

Type GlobalVarSymbol::getType() {
	return GLOBAL_VAR;
}
