#pragma once
#include "Core/Object.h"

namespace sda
{
    class IContext;

    // Factory interface
    class IFactory
    {
    public:
        // Create an object from a data
        ISerializable* create(IContext* context, boost::json::object& data);
        
        // Create an object
        virtual ISerializable* create(IContext* context, ObjectId* id, const std::string& collection, ObjectType type) = 0;
    };

    // Factory stores factories and can create objects
    class Factory : public IFactory
    {
        std::list<std::unique_ptr<IFactory>> m_factories;
    public:
        // Create an object
        ISerializable* create(IContext* context, ObjectId* id, const std::string& collection, ObjectType type) override;

        // Add a new factory
        void add(std::unique_ptr<IFactory> factory);
    };
};