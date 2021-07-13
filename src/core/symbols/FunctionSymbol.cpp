#include "FunctionSymbol.h"
#include <Function.h>

using namespace CE;

Symbol::FunctionSymbol::FunctionSymbol(SymbolManager* manager, int64_t offset, DataType::IFunctionSignature* funcSignature, const std::string& name, const std::string& comment)
	: m_funcSignature(funcSignature), GlobalVarSymbol(manager, offset, GetUnit(funcSignature), name, comment)
{}

Symbol::Type Symbol::FunctionSymbol::getType() {
	return FUNCTION;
}

int Symbol::FunctionSymbol::getSize() {
	return 1;
}

Function* Symbol::FunctionSymbol::getFunction() const
{
	return m_function;
}

DataType::IFunctionSignature* Symbol::FunctionSymbol::getSignature() const
{
	return m_funcSignature;
}

void Symbol::FunctionSymbol::setFunction(Function* function) {
	m_function = function;
}
