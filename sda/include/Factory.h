#pragma once
#include "Core/Object.h"

namespace sda
{
    // Factory interface
    class IFactory
    {
    public:
        // Create an object
        virtual IObject* create(Context* context, ObjectId* id, const std::string& collection, ObjectType type) = 0;
    };

    // Factory stores factories and can create objects
    class Factory : public IFactory
    {
        std::list<std::unique_ptr<IFactory>> m_factories;
    public:
        // Create an object
        IObject* create(Context* context, ObjectId* id, const std::string& collection, ObjectType type) override;

        // Add a new factory
        void add(std::unique_ptr<IFactory> factory);
    };
};