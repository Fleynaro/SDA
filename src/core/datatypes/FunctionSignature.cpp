#include "FunctionSignature.h"
#include <managers/TypeManager.h>
#include <managers/SymbolManager.h>
#include <decompiler/PCode/DecPCode.h>

using namespace CE;
using namespace DataType;
using namespace Decompiler;

Symbol::FuncParameterSymbol ParameterList::createParameter(const std::string& name, DataTypePtr dataType,
                                                           const std::string& comment) {
	const auto symbolManager = m_funcSignature->getTypeManager()->getProject()->getSymbolManager();
	auto param = Symbol::FuncParameterSymbol(symbolManager, dataType, name, comment);
	param.m_paramsList = this;
	return param;
}

void ParameterList::updateParameterStorages() {
	m_paramInfos.clear();

	for (const auto pair : m_customStorages) {
		const auto paramIdx = pair.first;
		const auto storage = pair.second;
		if (paramIdx >= 1 && paramIdx <= m_params.size()) {
			const auto paramSize = m_params[paramIdx - 1].getDataType()->getSize();
			m_paramInfos.emplace_back(paramIdx, paramSize, storage);
		}
		else if (paramIdx == 0) {
			//if it is return
			m_paramInfos.emplace_back(0, m_returnType->getSize(), storage);
		}
	}

	//calling conventions
	if (m_callingConvetion == FASTCALL) {
		//parameters
		int paramIdx = 1;
		for (auto& param : m_params) {
			const auto paramType = param.getDataType();
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
		auto retType = m_returnType;
		if (retType->getSize() != 0x0) {
			const auto regId = !retType->isFloatingPoint() ? ZYDIS_REGISTER_RAX : ZYDIS_REGISTER_ZMM0;
			const auto storage = Storage(Storage::STORAGE_REGISTER, regId, 0x0);
			m_paramInfos.emplace_back(retType->getSize(), storage);
		}
	}
}

FunctionSignature::FunctionSignature(TypeManager* typeManager, const std::string& name, const std::string& comment, CallingConvetion callingConvetion)
	: UserDefinedType(typeManager, name, comment),
	m_paramsList(this, callingConvetion, GetUnit(typeManager->getFactory().getDefaultReturnType()))
{}

AbstractType::Group FunctionSignature::getGroup() {
	return Group::FunctionSignature;
}

int FunctionSignature::getSize() {
	return sizeof(std::uintptr_t);
}

std::string FunctionSignature::getDisplayName() {
	return getSigName();
}

bool FunctionSignature::isAuto() {
	return m_isAuto;
}

void FunctionSignature::setAuto(bool toggle) {
	m_isAuto = toggle;
}

std::string FunctionSignature::getSigName() {
	std::string name = m_paramsList.getReturnType()->getDisplayName() + " " + getName() + "(";

	for (int i = 0; i < m_paramsList.getParamsCount(); i++) {
		name += m_paramsList[i]->getDataType()->getDisplayName() + " " + m_paramsList[i]->getName() + ", ";
	}
	if (m_paramsList.getParamsCount() > 0) {
		name.pop_back();
		name.pop_back();
	}
	return name + ")";
}
ParameterList& FunctionSignature::getParameters() {
	return m_paramsList;
}

void FunctionSignature::setParameters(const ParameterList& params) {
	m_paramsList = params;
}