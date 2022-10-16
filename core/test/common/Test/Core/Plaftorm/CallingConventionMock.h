#pragma once
#include <gmock/gmock.h>
#include "SDA/Core/Platform/CallingConvention.h"
#include "Test/Core/Utils/SerializationMock.h"

namespace sda::test
{
    class CallingConventionMock : public CallingConvention
    {
    public:
        MOCK_METHOD(std::string, getName, (), (const, override));

        MOCK_METHOD(Map, getStorages, (const SignatureDataType* signatureDt), (const, override));

        MOCK_METHOD(bool, getStorageInfo, (const Storage& storage, StorageInfo& storageInfo), (const, override));
    };
};