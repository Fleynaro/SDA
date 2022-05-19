#pragma once
#include <map>
#include <memory>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>
#include "Core/Serialization.h"
#include "Core/Destroy.h"

namespace sda
{
    // Base class for all domain objects
    class Object : public virtual ISerializable
    {
    public:
        using Id = boost::uuids::uuid;

    private:
        Id m_id;
        std::string m_name = "";
        std::string m_comment = "";
        bool m_temporary = false;

    public:
        Object(Id* id, const std::string& name);

        // Get the unique identifier of the object
        Id getId() const;

        // Get the name of the object
        const std::string& getName() const;

        // Set the name of the object
        virtual void setName(const std::string& name);

        // Get the comment of the object
        const std::string& getComment() const;

        // Set the comment of the object
        virtual void setComment(const std::string& comment);

        // Check if the object is temporary
        bool isTemporary() const;

        // Set the object as temporary
        void setTemporary(bool temporary);

        // Serialize the object id to a json string
        boost::json::string serializeId() const;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};