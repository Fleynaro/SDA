#include "DecStorage.h"

#include <utility>

using namespace CE;
using namespace Decompiler;
using namespace PCode;

FunctionCallInfo::FunctionCallInfo(std::list<ParameterInfo> paramInfos)
	: m_paramInfos(std::move(paramInfos))
{}

std::list<ParameterInfo>& FunctionCallInfo::getParamInfos() {
	return m_paramInfos;
}

ParameterInfo FunctionCallInfo::findParamInfoByIndex(int idx) {
	for (auto& paramInfo : getParamInfos()) {
		if (idx == paramInfo.getIndex()) {
			return paramInfo;
		}
	}
	return ParameterInfo();
}

ReturnInfo FunctionCallInfo::getReturnInfo() {
	return findParamInfoByIndex(0);
}

int FunctionCallInfo::findIndex(const Register& reg, int64_t offset) {
	for (auto paramInfo : m_paramInfos) {
		auto& storage = paramInfo.m_storage;
		const auto storageRegIndex = static_cast<int>(storage.getOffset()) / 8;
		if (storage.getType() == Storage::STORAGE_REGISTER && (reg.getId() == storage.getRegId() && reg.getIndex() == storageRegIndex) ||
			(offset == storage.getOffset() && (storage.getType() == Storage::STORAGE_STACK && reg.getType() == Register::Type::StackPointer ||
				storage.getType() == Storage::STORAGE_GLOBAL && reg.getType() == Register::Type::InstructionPointer))) {
			return paramInfo.getIndex();
		}
	}
	return -1;
}

Storage::Storage(StorageType storageType, int regGenericId, int64_t offset)
	: m_storageType(storageType), m_regId(regGenericId << 8), m_offset(offset)
{}

Storage::Storage(const Register& reg)
	: m_storageType(STORAGE_REGISTER), m_regId(reg.getId()), m_offset(reg.getOffset())
{}

Storage::StorageType Storage::getType() const
{
	return m_storageType;
}

int Storage::getRegId() const {
	return m_regId;
}

int Storage::getRegGenericId() const {
	return m_regId >> 8;
}

int64_t Storage::getOffset() const {
	return m_offset;
}

int ParameterInfo::getIndex() const {
	return m_index;
}

bool Decompiler::GetIndex_FASTCALL(const Register& reg, int64_t offset, int& paramIdx, bool& isFloating) {
	if (reg.getType() == Register::Type::StackPointer) {
		paramIdx = static_cast<int>(offset) / 0x8 - 5 + 1;
		isFloating = false;
		return true;
	}
	std::map<RegisterId, int> regToParamId = {
		std::pair(ZYDIS_REGISTER_RCX, 1),
		std::pair(ZYDIS_REGISTER_XMM0, 1),
		std::pair(ZYDIS_REGISTER_RDX, 2),
		std::pair(ZYDIS_REGISTER_XMM1, 2),
		std::pair(ZYDIS_REGISTER_R8, 3),
		std::pair(ZYDIS_REGISTER_XMM2, 3),
		std::pair(ZYDIS_REGISTER_R9, 4),
		std::pair(ZYDIS_REGISTER_XMM3, 4),
	};
	const auto it = regToParamId.find(reg.getGenericId());
	if (it != regToParamId.end()) {
		paramIdx = it->second;
		isFloating = reg.isVector();
		return true;
	}
	return false;
}
