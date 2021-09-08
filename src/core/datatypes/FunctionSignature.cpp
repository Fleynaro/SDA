#include "FunctionSignature.h"
#include <managers/TypeManager.h>
#include <managers/SymbolManager.h>
#include <decompiler/PCode/DecPCode.h>

using namespace CE;
using namespace DataType;
using namespace Decompiler;

FunctionSignature::FunctionSignature(TypeManager* typeManager, const std::string& name, const std::string& comment, CallingConvetion callingConvetion)
	: UserDefinedType(typeManager, name, comment), m_callingConvetion(callingConvetion), m_parameters(this)
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

CallingConvetion FunctionSignature::getCallingConvetion() {
	return m_callingConvetion;
}

std::list<std::pair<int, Storage>>& FunctionSignature::getCustomStorages() {
	return m_customStorages;
}

std::string FunctionSignature::getSigName() {
	std::string name = getReturnType()->getDisplayName() + " " + getName() + "(";

	for (int i = 0; i < m_parameters.getParamsCount(); i++) {
		name += m_parameters[i]->getDataType()->getDisplayName() + " " + m_parameters[i]->getName() + ", ";
	}
	if (m_parameters.getParamsCount() > 0) {
		name.pop_back();
		name.pop_back();
	}
	return name + ")";
}

void FunctionSignature::setReturnType(DataTypePtr returnType) {
	m_returnType = returnType;
	m_isReturnStorageChanged = true;
}

DataTypePtr FunctionSignature::getReturnType() {
	return m_returnType;
}

ParameterList& FunctionSignature::getParameters() {
	return m_parameters;
}

void FunctionSignature::addParameter(const std::string& name, DataTypePtr dataType, const std::string& comment) {
	auto symbolManager = getTypeManager()->getProject()->getSymbolManager();
	const auto paramSymbol = symbolManager->getFactory().createFuncParameterSymbol(dataType, name, comment);
	m_parameters.addParameter(paramSymbol);
}

FunctionCallInfo FunctionSignature::getCallInfo() {
	if(m_isReturnStorageChanged || m_parameters.m_areParamStoragesChanged) {
		updateParameterStorages();
		m_isReturnStorageChanged = false;
		m_parameters.m_areParamStoragesChanged = false;
	}
	return FunctionCallInfo(m_paramInfos);
}

void FunctionSignature::updateParameterStorages() {
	m_paramInfos.clear();
	
	for (const auto pair : getCustomStorages()) {
		const auto paramIdx = pair.first;
		const auto storage = pair.second;
		if (paramIdx >= 1 && paramIdx <= m_parameters.getParamsCount()) {
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
		for (int i = 0; i < m_parameters.getParamsCount(); i++) {
			const auto paramType = m_parameters[i]->getDataType();
			if (paramIdx >= 1 && paramIdx <= 4) {
				static std::map<int, std::pair<PCode::RegisterId, PCode::RegisterId>> paramToReg = {
							std::pair(1, std::pair(ZYDIS_REGISTER_RCX, ZYDIS_REGISTER_XMM0)), //todo: replace xmm -> zmm
							std::pair(2, std::pair(ZYDIS_REGISTER_RDX, ZYDIS_REGISTER_XMM1)),
							std::pair(3, std::pair(ZYDIS_REGISTER_R8, ZYDIS_REGISTER_XMM2)),
							std::pair(4, std::pair(ZYDIS_REGISTER_R9, ZYDIS_REGISTER_XMM3))
				};
				const auto it = paramToReg.find(paramIdx);
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
		const auto retType = getReturnType();
		if (retType->getSize() != 0x0) {
			const auto regId = !retType->isFloatingPoint() ? ZYDIS_REGISTER_RAX : ZYDIS_REGISTER_XMM0;
			const auto storage = Storage(Storage::STORAGE_REGISTER, regId, 0x0);
			m_paramInfos.emplace_back(0, retType->getSize(), storage);
		}
	}
}

IFunctionSignature* FunctionSignature::clone() {
	const auto funcSig = new FunctionSignature(getTypeManager(), getName(), getComment(), getCallingConvetion());
	for (int i = 0; i < m_parameters.getParamsCount(); i ++) {
		funcSig->m_parameters.addParameter(m_parameters[i]->clone());
	}
	funcSig->m_paramInfos = m_paramInfos;
	funcSig->m_customStorages = m_customStorages;
	funcSig->m_returnType = m_returnType;
	return funcSig;
}

void FunctionSignature::apply(IFunctionSignature* funcSignature) {
	m_parameters = funcSignature->getParameters();
	funcSignature->getParameters().clear();
	setReturnType(funcSignature->getReturnType());
	m_parameters.m_funcSignature = this;
	m_customStorages = funcSignature->getCustomStorages();
	updateParameterStorages();

	for (int i = 0; i < m_parameters.getParamsCount(); i++) {
		m_parameters[i]->m_signature = this;
	}
}
