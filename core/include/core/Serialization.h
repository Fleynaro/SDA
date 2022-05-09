#pragma once
#include <boost/json.hpp>

namespace sda
{
    using JsonData = boost::json::value;

    class ISerializable
    {
    public:
        virtual void serialize(JsonData& json) const = 0;

        virtual void deserialize(const JsonData& json) = 0;
    };
};