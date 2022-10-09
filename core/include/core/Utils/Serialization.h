#pragma once
#include <boost/json.hpp>
#include "Core/Utils/BoostJsonNumber.h"

namespace utils
{
    // Interface for objects that can be serialized
    class ISerializable
    {
    public:
        // Serialize the object
        virtual void serialize(boost::json::object& data) const = 0;

        // Deserialize the object
        virtual void deserialize(boost::json::object& data) {}
    };
};