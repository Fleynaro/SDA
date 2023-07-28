#pragma once
#include "SDA/Core/Context.h"

namespace sda
{
    class Factory
    {
        Context* m_context;
    public:
        Factory(Context* context);

        // Create an object
        ContextObject* create(boost::uuids::uuid* id, const std::string& className, boost::json::object& data);

        // Create an object from a data
        ContextObject* create(boost::json::object& data);
    };
};