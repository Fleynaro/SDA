#pragma once
#include <boost/uuid/uuid.hpp>
#include "SDA/Core/Utils/Serialization.h"

namespace sda
{
    // Base class for all domain objects
    class Object : public utils::ISerializable
    {
    public:
        using Id = boost::uuids::uuid;

        enum class ModState {
            Before,
            After
        };

    private:
        Id m_id;

    public:
        Object(Id* id);

        virtual ~Object() = default;

        // Get the unique identifier of the object
        Id getId() const;

        // Serialize the object id to a json string
        boost::json::string serializeId() const;

        void serialize(boost::json::object& data) const override;
    };
};