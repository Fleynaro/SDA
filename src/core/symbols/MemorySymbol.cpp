#include "MemorySymbol.h"

using namespace CE;
using namespace CE::Symbol;

CE::Symbol::LocalStackVarSymbol::LocalStackVarSymbol(SymbolManager* manager, int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment)
	: AbstractMemorySymbol(manager, offset, type, name, comment)
{}

Type CE::Symbol::LocalStackVarSymbol::getType() {
	return LOCAL_STACK_VAR;
}

Decompiler::Storage CE::Symbol::LocalStackVarSymbol::getStorage() {
	return Decompiler::Storage(Decompiler::Storage::STORAGE_STACK, 0, getOffset());
}

CE::Symbol::AbstractMemorySymbol::AbstractMemorySymbol(SymbolManager* manager, int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment)
	: AbstractSymbol(manager, type, name, comment), m_offset(offset)
{}

int64_t CE::Symbol::AbstractMemorySymbol::getOffset() const
{
	return m_offset;
}

void CE::Symbol::AbstractMemorySymbol::setOffset(int64_t offset) {
	m_offset = offset;
}

CE::Symbol::GlobalVarSymbol::GlobalVarSymbol(SymbolManager* manager, int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment)
	: AbstractMemorySymbol(manager, offset, type, name, comment)
{}

Type CE::Symbol::GlobalVarSymbol::getType() {
	return GLOBAL_VAR;
}

Decompiler::Storage CE::Symbol::GlobalVarSymbol::getStorage() {
	return Decompiler::Storage(Decompiler::Storage::STORAGE_GLOBAL, 0, getOffset());
}
