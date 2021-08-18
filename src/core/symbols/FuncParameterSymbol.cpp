#include "FuncParameterSymbol.h"
#include <datatypes/FunctionSignature.h>

using namespace CE;
using namespace Symbol;

Decompiler::Storage FuncParameterSymbol::getStorage() {
	return getParamInfo().m_storage;
}

int FuncParameterSymbol::getParamIdx() const {
	return m_paramIdx;
}

Type FuncParameterSymbol::getType() {
	return FUNC_PARAMETER;
}

Decompiler::ParameterInfo FuncParameterSymbol::getParamInfo() const {
	return m_signature->getCallInfo().findParamInfoByIndex(m_paramIdx);
}