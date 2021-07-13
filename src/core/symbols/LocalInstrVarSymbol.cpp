#include "LocalInstrVarSymbol.h"

using namespace CE;
using namespace Symbol;

LocalInstrVarSymbol::LocalInstrVarSymbol(SymbolManager* manager, DataTypePtr type, const std::string& name, const std::string& comment)
	: AbstractSymbol(manager, type, name, comment)
{}

Type LocalInstrVarSymbol::getType() {
	return LOCAL_INSTR_VAR;
}
