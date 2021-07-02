#include "FunctionSymbol.h"
#include <Function.h>

using namespace CE;

CE::Symbol::FunctionSymbol::FunctionSymbol(SymbolManager* manager, int64_t offset, DataType::IFunctionSignature* funcSignature, const std::string& name, const std::string& comment)
	: m_funcSignature(funcSignature), GlobalVarSymbol(manager, offset, DataType::GetUnit(funcSignature), name, comment)
{}

Symbol::Type CE::Symbol::FunctionSymbol::getType() {
	return FUNCTION;
}

int CE::Symbol::FunctionSymbol::getSize() {
	return 1;
}

Function* CE::Symbol::FunctionSymbol::getFunction() {
	return m_function;
}

DataType::IFunctionSignature* CE::Symbol::FunctionSymbol::getSignature() {
	return m_funcSignature;
}

void CE::Symbol::FunctionSymbol::setFunction(Function* function) {
	m_function = function;
}
