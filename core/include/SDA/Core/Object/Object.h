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
        std::list<Object*> m_parents;

    public:
        Object(Id* id);

        virtual ~Object() = default;

        // Get the unique identifier of the object
        Id getId() const;

        // Get the list of parents of the object
        const std::list<Object*>& getParents() const;

        // Add a parent to the object
        void addParent(Object* parent);

        // Remove a parent from the object
        void removeParent(Object* parent);

        // Serialize the object id to a json string
        boost::json::string serializeId() const;

        void serialize(boost::json::object& data) const override;
    };
};