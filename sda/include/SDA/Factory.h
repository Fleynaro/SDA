#pragma once
#include <boost/uuid/uuid.hpp>
#include "SDA/Core/Utils/Serialization.h"

namespace sda
{
    // Factory interface
    class IFactory
    {
    public:
        // Create an object from a data
        utils::ISerializable* create(boost::json::object& data);
        
        // Create an object
        virtual utils::ISerializable* create(boost::uuids::uuid* id, const std::string& collection, boost::json::object& data) = 0;
    };

    class Context;
    
    // Factory stores factories and can create objects
    class Factory : public IFactory
    {
        Context* m_context;
        std::list<std::unique_ptr<IFactory>> m_factories;
    public:
        Factory(Context* context);

        // Create an object
        utils::ISerializable* create(boost::uuids::uuid* id, const std::string& collection, boost::json::object& data) override;

        // Add a new factory
        void add(std::unique_ptr<IFactory> factory);
    };
};