#pragma once
#include "Database.h"

namespace sda
{
    class Context;
    class IFactory;
    class ISerializable;

    // Loader allows to load all collections from a database
    class Loader
    {
        Database* m_database;
        Context* m_context;
        IFactory* m_factory;
        std::list<std::pair<ISerializable*, boost::json::object>> m_objects;
    public:
        Loader(Database* database, Context* context, IFactory* factory);

        void load();

    private:
        // Load all objects from a collection
        void loadCollection(const std::string& name);
    };
};