#include "FuncParameterSymbol.h"
#include <Code/Type/FunctionSignature.h>

using namespace CE;
using namespace CE::Symbol;

Decompiler::Storage CE::Symbol::FuncParameterSymbol::getStorage() {
	auto paramIdx = getParamIdx();
	for (auto& paramInfo : m_signature->getCallInfo().getParamInfos()) {
		if (paramIdx == paramInfo.getIndex()) {
			return paramInfo.m_storage;
		}
	}
	return Decompiler::Storage();
}
