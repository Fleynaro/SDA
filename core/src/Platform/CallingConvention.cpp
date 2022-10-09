#include "Core/Platform/CallingConvention.h"

using namespace sda;

bool CallingConvention::Storage::operator<(const CallingConvention::Storage& other) const {
    if (useType == other.useType) {
        if (registerId == other.registerId) {
            return offset < other.offset;
        }
        return registerId < other.registerId;
    }
    return useType < other.useType;
}

CustomCallingConvention::CustomCallingConvention(Map storages)
    : m_storages(storages)
{}

std::string CustomCallingConvention::getName() const {
    return Name;
}

CallingConvention::Map CustomCallingConvention::getStorages(const SignatureDataType* signatureDt) const {
    return m_storages;
}

bool CustomCallingConvention::getStorageInfo(const Storage& storage, StorageInfo& storageInfo) const {
    auto it = m_storages.find(storage);
    if (it == m_storages.end())
        return false;
    storageInfo = it->second;
    return true;
}

void CustomCallingConvention::serialize(boost::json::object& data) const {
    data["type"] = Name;
    boost::json::array storages;
    for (auto& pair : m_storages) {
        boost::json::object storage;
        storage["s_use_type"] = pair.first.useType;
        storage["s_register_id"] = pair.first.registerId;
        storage["s_offset"] = pair.first.offset;

        storage["si_type"] = pair.second.type;
        storage["si_param_idx"] = pair.second.paramIdx;
        storage["si_is_storing_float"] = pair.second.isStoringFloat;
        storages.push_back(storage);
    }
    data["storages"] = storages;
}

void CustomCallingConvention::deserialize(boost::json::object& data) {
    const auto& storages = data["storages"].get_array();
    for (const auto& value : storages) {
        auto obj = value.get_object();

        Storage storage;
        storage.useType = utils::get_number<Storage::UseType>(obj["s_use_type"]);
        storage.registerId = utils::get_number<size_t>(obj["s_register_id"]);
        storage.offset = utils::get_number<size_t>(obj["s_offset"]);

        StorageInfo storageInfo;
        storageInfo.type = utils::get_number<StorageInfo::Type>(obj["si_type"]);
        storageInfo.paramIdx = utils::get_number<size_t>(obj["si_param_idx"]);
        storageInfo.isStoringFloat = obj["si_is_storing_float"].get_bool();

        m_storages[storage] = storageInfo;
    }
}