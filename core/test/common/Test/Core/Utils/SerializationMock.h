#pragma once
#include <gmock/gmock.h>
#include "SDA/Core/Utils/Serialization.h"

namespace sda::test
{
    class SerializationMock : public utils::ISerializable
    {
    public:
        MOCK_METHOD(void, serialize, (boost::json::object& data), (const, override));
        
        MOCK_METHOD(void, deserialize, (boost::json::object& data), (override));
    };
};