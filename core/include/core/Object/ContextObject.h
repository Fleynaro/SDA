#pragma once
#include "Object.h"
#include "Core/Destroy.h"

namespace sda
{
    class Context;

    // Base class for all domain objects of context
    class ContextObject : public Object, public IDestroyable
    {
        std::string m_name = "";
        std::string m_comment = "";
    protected:
        Context* m_context;

        // Notify that an object has been modified
        virtual void notifyModified(Object::ModState state);
        
    public:
        ContextObject(Context* context, Object::Id* id, const std::string& name);

        // Get the name of the object
        const std::string& getName() const;

        // Set the name of the object
        void setName(const std::string& name);

        // Get the comment of the object
        const std::string& getComment() const;

        // Set the comment of the object
        void setComment(const std::string& comment);

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};