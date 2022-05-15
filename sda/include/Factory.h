#pragma once
#include "Core/Object.h"

namespace sda
{
    // Factory interface
    class IFactory
    {
    public:
        // Create an object from a data
        ISerializable* create(boost::json::object& data);
        
        // Create an object
        virtual ISerializable* create(ObjectId* id, const std::string& collection, ObjectType type) = 0;
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
        ISerializable* create(ObjectId* id, const std::string& collection, ObjectType type) override;

        // Add a new factory
        void add(std::unique_ptr<IFactory> factory);
    };
};