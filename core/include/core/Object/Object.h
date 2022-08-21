#pragma once
#include <boost/uuid/uuid.hpp>
#include "Core/Serialization.h"

namespace sda
{
    // Base class for all domain objects
    class Object : public ISerializable
    {
    public:
        using Id = boost::uuids::uuid;

        enum class ModState {
            Before,
            After
        };

    private:
        Id m_id;
        bool m_temporary = false;

    public:
        Object(Id* id);

        virtual ~Object() = default;

        // Get the unique identifier of the object
        Id getId() const;

        // Set the object as temporary
        void setTemporary(bool temporary);

        // Serialize the object id to a json string
        boost::json::string serializeId() const;

        void serialize(boost::json::object& data) const override;
    };
};