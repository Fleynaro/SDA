#include "FuncParameterSymbol.h"
#include <datatypes/FunctionSignature.h>

using namespace CE;
using namespace Symbol;

int FuncParameterSymbol::getParamIdx() const
{
	return m_paramIdx;
}

Type FuncParameterSymbol::getType() {
	return FUNC_PARAMETER;
}

Decompiler::Storage FuncParameterSymbol::getStorage() {
	const auto paramIdx = getParamIdx();
	auto callInfo = m_signature->getCallInfo();
	for (auto& paramInfo : callInfo.getParamInfos()) {
		if (paramIdx == paramInfo.getIndex()) {
			return paramInfo.m_storage;
		}
	}
	return Decompiler::Storage();
}

void FuncParameterSymbol::setFuncSignature(DataType::IFunctionSignature* signature) {
	m_signature = signature;
}
