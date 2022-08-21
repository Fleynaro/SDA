#pragma once
#include "Core/Platform/CallingConvention.h"

namespace sda::platform
{
    class FastcallCallingConvention : public CallingConvention, public ISerializable
    {
    public:
        static inline const std::string Name = "fastcall";

        std::string getName() const override;

        Map getStorages(const SignatureDataType* signatureDt) const override;

        bool getStorageInfo(const Storage& storage, StorageInfo& storageInfo) const override;

        void serialize(boost::json::object& data) const override;
    };
};
