#pragma once
#include "Object.h"

namespace sda
{
    class Context;

    // Base class for all domain objects of context
    class ContextObject : public Object, public virtual IDestroyable
    {
    protected:
        Context* m_context;
        
    public:
        ContextObject(Context* context, Object::Id* id, const std::string& name);

        // Set the name of the object
        void setName(const std::string& name) override;

        // Set the comment of the object
        void setComment(const std::string& comment) override;

        void deserialize(boost::json::object& data) override;
    };
};