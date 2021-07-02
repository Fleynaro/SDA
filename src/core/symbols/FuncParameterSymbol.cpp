#include "FuncParameterSymbol.h"
#include <datatypes/FunctionSignature.h>

using namespace CE;
using namespace CE::Symbol;

int CE::Symbol::FuncParameterSymbol::getParamIdx() {
	return m_paramIdx;
}

Type CE::Symbol::FuncParameterSymbol::getType() {
	return FUNC_PARAMETER;
}

Decompiler::Storage CE::Symbol::FuncParameterSymbol::getStorage() {
	auto paramIdx = getParamIdx();
	for (auto& paramInfo : m_signature->getCallInfo().getParamInfos()) {
		if (paramIdx == paramInfo.getIndex()) {
			return paramInfo.m_storage;
		}
	}
	return Decompiler::Storage();
}

void CE::Symbol::FuncParameterSymbol::setFuncSignature(DataType::IFunctionSignature* signature) {
	m_signature = signature;
}
