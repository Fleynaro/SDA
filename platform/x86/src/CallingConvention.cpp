#include "Platform/X86/CallingConvention.h"
#include "Platform/X86/RegisterHelper.h"
#include "Core/DataType/SignatureDataType.h"
#include <Zydis/Zydis.h>
#include <algorithm>

using namespace sda;
using namespace sda::platform;

std::string FastcallCallingConvention::getName() const {
    return Name;
}

CallingConvention::Map FastcallCallingConvention::getStorages(const SignatureDataType* signatureDt) const {
    static const std::vector<std::pair<size_t, size_t>> ParamRegisters = {
        std::make_pair(ZYDIS_REGISTER_RCX, ZYDIS_REGISTER_XMM0),
        std::make_pair(ZYDIS_REGISTER_RDX, ZYDIS_REGISTER_XMM1),
        std::make_pair(ZYDIS_REGISTER_R8, ZYDIS_REGISTER_XMM2),
        std::make_pair(ZYDIS_REGISTER_R9, ZYDIS_REGISTER_XMM3)
    };

    Map storages;
    auto params = signatureDt->getParameters();
    for (size_t i = 0; i < std::min(size_t(4), params.size()); i ++) {
        auto param = params[i];
        StorageInfo storageInfo = { StorageInfo::Parameter, i };
        if (param->getDataType()->isScalar(ScalarType::FloatingPoint)) {
            storageInfo.isStoringFloat = true;
            storages[{Storage::Read, ParamRegisters[i].second}] = storageInfo;
        } else {
            storages[{Storage::Read, ParamRegisters[i].first}] = storageInfo;
        }
    }

    for (size_t i = 4; i < params.size(); i ++) {
        StorageInfo storageInfo = { StorageInfo::Parameter, i };
        storages[{Storage::Read, Register::StackPointerId, 0x8 * i}] = storageInfo;
    }

    auto returnDt = signatureDt->getReturnType();
    if (!returnDt->isVoid()) {
        StorageInfo storageInfo = { StorageInfo::Return };
        if (returnDt->isScalar(ScalarType::FloatingPoint)) {
            storageInfo.isStoringFloat = true;
            storages[{Storage::Write, ZYDIS_REGISTER_XMM0}] = storageInfo;
        } else {
            storages[{Storage::Write, ZYDIS_REGISTER_RAX}] = storageInfo;
        }
    }

    return storages;
}

bool FastcallCallingConvention::getStorageInfo(const Storage& storage, StorageInfo& storageInfo) const {
    static const std::map<size_t, size_t> RegToParamIdx = {
		std::make_pair(ZYDIS_REGISTER_RCX, 1),
		std::make_pair(ZYDIS_REGISTER_XMM0, 1),
		std::make_pair(ZYDIS_REGISTER_RDX, 2),
		std::make_pair(ZYDIS_REGISTER_XMM1, 2),
		std::make_pair(ZYDIS_REGISTER_R8, 3),
		std::make_pair(ZYDIS_REGISTER_XMM2, 3),
		std::make_pair(ZYDIS_REGISTER_R9, 4),
		std::make_pair(ZYDIS_REGISTER_XMM3, 4),
	};

    if (storage.registerId == Register::StackPointerId) {
		storageInfo.type = StorageInfo::Parameter;
        storageInfo.paramIdx = storage.offset / 0x8 - 5 + 1;
		return true;
	}

    const auto it = RegToParamIdx.find(storage.registerId);
	if (it != RegToParamIdx.end()) {
        auto regType = RegisterHelperX86().getRegisterType(storage.registerId);
        storageInfo.type = StorageInfo::Parameter;
        storageInfo.paramIdx = it->second;
        storageInfo.isStoringFloat = regType == Register::Vector;
		return true;
	}

    if (storage.registerId == ZYDIS_REGISTER_RAX ||
        storage.registerId == ZYDIS_REGISTER_XMM0) {
        storageInfo.type = StorageInfo::Return;
        storageInfo.isStoringFloat = storage.registerId == ZYDIS_REGISTER_XMM0;
        return true;
    }
	return false;
}

void FastcallCallingConvention::serialize(boost::json::object& data) const {
    data["type"] = Name;
}