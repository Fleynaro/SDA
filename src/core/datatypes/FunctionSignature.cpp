#include "FunctionSignature.h"
#include <managers/TypeManager.h>
#include <managers/SymbolManager.h>
#include <decompiler/PCode/DecPCode.h>

using namespace CE;
using namespace DataType;
using namespace Decompiler;

FunctionSignature::FunctionSignature(TypeManager* typeManager, const std::string& name, const std::string& comment, CallingConvetion callingConvetion)
	: UserDefinedType(typeManager, name, comment), m_callingConvetion(callingConvetion)
{
	setReturnType(GetUnit(typeManager->getFactory().getDefaultReturnType()));
}

AbstractType::Group FunctionSignature::getGroup() {
	return Group::FunctionSignature;
}

int FunctionSignature::getSize() {
	return sizeof(std::uintptr_t);
}

std::string FunctionSignature::getDisplayName() {
	return getSigName();
}

FunctionSignature::CallingConvetion FunctionSignature::getCallingConvetion() {
	return m_callingConvetion;
}

std::list<std::pair<int, Storage>>& FunctionSignature::getCustomStorages() {
	return m_customStorages;
}

std::string FunctionSignature::getSigName() {
	std::string name = getReturnType()->getDisplayName() + " " + getName() + "(";

	auto& argList = getParameters();
	for (int i = 0; i < argList.size(); i++) {
		name += argList[i]->getDataType()->getDisplayName() + " " + argList[i]->getName() + ", ";
	}
	if (argList.size() > 0) {
		name.pop_back();
		name.pop_back();
	}
	return name + ")";
}

void FunctionSignature::setReturnType(DataTypePtr returnType) {
	m_returnType = returnType;
	m_hasSignatureUpdated = true;
}

DataTypePtr FunctionSignature::getReturnType() {
	return m_returnType;
}

std::vector<Symbol::FuncParameterSymbol*>& FunctionSignature::getParameters() {
	return m_parameters;
}

void FunctionSignature::addParameter(Symbol::FuncParameterSymbol* symbol) {
	m_parameters.push_back(symbol);
	symbol->setFuncSignature(this);
	m_hasSignatureUpdated = true;
}

void FunctionSignature::addParameter(const std::string& name, DataTypePtr dataType, const std::string& comment) {
	auto symbolManager = getTypeManager()->getProject()->getSymbolManager();
	const auto paramIdx = static_cast<int>(m_parameters.size());
	const auto paramSymbol = symbolManager->getFactory().createFuncParameterSymbol(paramIdx, this, dataType, name, comment);
	addParameter(paramSymbol);
}

void FunctionSignature::removeLastParameter() {
	m_parameters.pop_back();
	m_hasSignatureUpdated = true;
}

void FunctionSignature::deleteAllParameters() {
	m_parameters.clear();
	m_hasSignatureUpdated = true;
}

FunctionCallInfo FunctionSignature::getCallInfo() {
	if (m_hasSignatureUpdated) {
		m_paramInfos.clear();
		updateParameterStorages();
		m_hasSignatureUpdated = false;
	}
	return FunctionCallInfo(m_paramInfos);
}

void FunctionSignature::updateParameterStorages() {
	for (const auto pair : getCustomStorages()) {
		const auto paramIdx = pair.first;
		const auto storage = pair.second;
		if (paramIdx >= 1 && paramIdx <= getParameters().size()) {
			const auto paramSize = getParameters()[paramIdx - 1]->getDataType()->getSize();
			m_paramInfos.emplace_back(paramIdx, paramSize, storage);
		}
		else if (paramIdx == 0) {
			//if it is return
			m_paramInfos.emplace_back(0, getReturnType()->getSize(), storage);
		}
	}

	//calling conventions
	if (getCallingConvetion() == FASTCALL) {
		//parameters
		int paramIdx = 1;
		for (auto param : getParameters()) {
			auto paramType = param->getDataType();
			if (paramIdx >= 1 && paramIdx <= 4) {
				static std::map<int, std::pair<PCode::RegisterId, PCode::RegisterId>> paramToReg = {
							std::pair(1, std::pair(ZYDIS_REGISTER_RCX, ZYDIS_REGISTER_ZMM0)),
							std::pair(2, std::pair(ZYDIS_REGISTER_RDX, ZYDIS_REGISTER_ZMM1)),
							std::pair(3, std::pair(ZYDIS_REGISTER_R8, ZYDIS_REGISTER_ZMM2)),
							std::pair(4, std::pair(ZYDIS_REGISTER_R9, ZYDIS_REGISTER_ZMM3))
				};
				auto it = paramToReg.find(paramIdx);
				if (it != paramToReg.end()) {
					auto& reg = it->second;
					const auto regId = !paramType->isFloatingPoint() ? reg.first : reg.second;
					const auto storage = Storage(Storage::STORAGE_REGISTER, regId, 0x0);
					m_paramInfos.emplace_back(paramIdx, paramType->getSize(), storage);
				}
			}
			else {
				const auto storage = Storage(Storage::STORAGE_STACK, ZYDIS_REGISTER_RSP, paramIdx * 0x8);
				m_paramInfos.emplace_back(paramIdx, paramType->getSize(), storage);
			}

			paramIdx++;
		}

		//return
		auto retType = getReturnType();
		if (retType->getSize() != 0x0) {
			const auto regId = !retType->isFloatingPoint() ? ZYDIS_REGISTER_RAX : ZYDIS_REGISTER_ZMM0;
			const auto storage = Storage(Storage::STORAGE_REGISTER, regId, 0x0);
			m_paramInfos.emplace_back(retType->getSize(), storage);
		}
	}
}