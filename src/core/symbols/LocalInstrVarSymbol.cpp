#include "LocalInstrVarSymbol.h"

using namespace CE;
using namespace Symbol;

LocalInstrVarSymbol::LocalInstrVarSymbol(SymbolManager* manager, DataTypePtr type, const std::string& name, const std::string& comment)
	: AbstractSymbol(manager, type, name, comment)
{}

Decompiler::Storage LocalInstrVarSymbol::getStorage() {
	return Decompiler::Storage(Decompiler::Storage::STORAGE_REGISTER, 0, 0);
}

Type LocalInstrVarSymbol::getType() {
	return LOCAL_INSTR_VAR;
}
