#include "DecStorage.h"

#include <utility>

using namespace CE;
using namespace CE::Decompiler;
using namespace CE::Decompiler::PCode;

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
	return ParameterInfo(0, 0, Storage());
}

ReturnInfo FunctionCallInfo::getReturnInfo() {
	return findParamInfoByIndex(0);
}

int FunctionCallInfo::findIndex(const Register& reg, int64_t offset) {
	for (auto paramInfo : m_paramInfos) {
		auto& storage = paramInfo.m_storage;
		const auto storageRegIndex = static_cast<int>(storage.getOffset()) / 8;
		if (storage.getType() == Storage::STORAGE_REGISTER && (reg.getGenericId() == storage.getRegisterId() && reg.getIndex() == storageRegIndex) ||
			(offset == storage.getOffset() && (storage.getType() == Storage::STORAGE_STACK && reg.getType() == Register::Type::StackPointer ||
				storage.getType() == Storage::STORAGE_GLOBAL && reg.getType() == Register::Type::InstructionPointer))) {
			return paramInfo.getIndex();
		}
	}
	return -1;
}

Storage::Storage(StorageType storageType, int registerId, int64_t offset)
	: m_storageType(storageType), m_registerId(registerId), m_offset(offset)
{}

Storage::StorageType Storage::getType() const
{
	return m_storageType;
}

int Storage::getRegisterId() const
{
	return m_registerId;
}

int64_t Storage::getOffset() const
{
	return m_offset;
}

int ParameterInfo::getIndex() const
{
	return m_index;
}

int CE::Decompiler::GetIndex_FASTCALL(const PCode::Register& reg, int64_t offset) {
	if (reg.getType() == PCode::Register::Type::StackPointer) {
		return static_cast<int>(offset) / 0x8 - 5 + 1;
	}
	std::map<PCode::RegisterId, int> regToParamId = {
		std::pair(ZYDIS_REGISTER_RCX, 1),
		std::pair(ZYDIS_REGISTER_ZMM0, 1),
		std::pair(ZYDIS_REGISTER_RDX, 2),
		std::pair(ZYDIS_REGISTER_ZMM1, 2),
		std::pair(ZYDIS_REGISTER_R8, 3),
		std::pair(ZYDIS_REGISTER_ZMM2, 3),
		std::pair(ZYDIS_REGISTER_R9, 4),
		std::pair(ZYDIS_REGISTER_ZMM3, 4),
	};
	const auto it = regToParamId.find(reg.getId());
	if (it != regToParamId.end()) {
		return it->second;
	}
	return -1;
}
