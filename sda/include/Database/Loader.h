#pragma once
#include "Database.h"

namespace sda
{
    class Context;
    class ISerializable;

    // Loader allows to load all collections from a database
    class Loader
    {
        Database* m_database;
        Context* m_context;
        std::list<std::pair<ISerializable*, boost::json::object>> m_objects;
    public:
        Loader(Database* database, Context* context);

        void load();

    private:
        // Load all objects from a collection
        void loadCollection(const std::string& name, std::function<ISerializable*(int)> creator);
    };
};