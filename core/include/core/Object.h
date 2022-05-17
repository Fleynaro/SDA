#pragma once
#include <map>
#include <memory>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>
#include "Serialization.h"
#include "Destroy.h"

namespace sda
{
    using ObjectId = boost::uuids::uuid;
    using ObjectType = int;
    class Context;

    // Base class for all domain objects
    class Object : public virtual ISerializable
    {
        ObjectId m_id;
        std::string m_name = "";
        std::string m_comment = "";
        bool m_temporary = false;
    public:
        Object(ObjectId* id, const std::string& name);

        // Get the unique identifier of the object
        ObjectId getId() const;

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

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };

    // Base class for all domain objects of context
    class ContextObject : public Object, public virtual IDestroyable
    {
    protected:
        Context* m_context;
        
    public:
        ContextObject(Context* context, ObjectId* id, const std::string& name);

        // Set the name of the object
        void setName(const std::string& name) override;

        // Set the comment of the object
        void setComment(const std::string& comment) override;
    };
};