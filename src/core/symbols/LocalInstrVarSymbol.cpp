#include "LocalInstrVarSymbol.h"

using namespace CE;
using namespace CE::Symbol;

CE::Symbol::LocalInstrVarSymbol::LocalInstrVarSymbol(SymbolManager* manager, DataTypePtr type, const std::string& name, const std::string& comment)
	: AbstractSymbol(manager, type, name, comment)
{}

Type CE::Symbol::LocalInstrVarSymbol::getType() {
	return LOCAL_INSTR_VAR;
}
