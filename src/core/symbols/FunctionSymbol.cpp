#include "FunctionSymbol.h"
#include <Function.h>

using namespace CE;

CE::Symbol::FunctionSymbol::FunctionSymbol(SymbolManager* manager, int64_t offset, DataType::IFunctionSignature* funcSignature, const std::string& name, const std::string& comment)
	: m_funcSignature(funcSignature), GlobalVarSymbol(manager, offset, DataType::GetUnit(funcSignature), name, comment)
{}
