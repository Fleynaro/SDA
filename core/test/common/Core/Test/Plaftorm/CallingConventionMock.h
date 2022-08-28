#pragma once
#include <gmock/gmock.h>
#include "Core/Platform/CallingConvention.h"
#include "Core/Test/Utils/SerializationMock.h"

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