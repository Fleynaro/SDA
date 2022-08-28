#pragma once
#include "Database.h"
#include "Core/Utils/Serialization.h"

namespace sda
{
    class IFactory;

    // Loader allows to load all collections from a database
    class Loader
    {
        Database* m_database;
        IFactory* m_factory;
        std::list<std::pair<utils::ISerializable*, boost::json::object>> m_objects;
    public:
        Loader(Database* database, IFactory* factory);

        void load();

    private:
        // Load all objects from a collection
        void loadCollection(const std::string& name);
    };
};