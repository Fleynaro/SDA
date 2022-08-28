#pragma once
#include <gmock/gmock.h>
#include "Core/Serialization.h"

namespace sda::test
{
    class SerializationMock : public ISerializable
    {
    public:
        MOCK_METHOD(void, serialize, (boost::json::object& data), (const, override));
        
        MOCK_METHOD(void, deserialize, (boost::json::object& data), (override));
    };
};