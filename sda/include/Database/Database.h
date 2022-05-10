#pragma once
#include <list>
#include <map>
#include <filesystem>
#include <sqlite3.h>
#include <boost/json.hpp>

namespace sda
{
    // Schema describes the structure of the database
    class Schema
    {
    public:
        struct Field {
            std::string name;
            bool isKey = false;
        };

        struct Collection {
            std::string name;
            std::list<Field> fields;
        };

        Schema(const std::list<Collection>& collections);

        // Get list of collections
        const std::list<Collection>& getCollections() const;

    private:
        std::list<Collection> m_collections;
    };

    class Collection;

    // Database allows to store and retrieve data
    class Database
    {
        std::filesystem::path m_path;
        std::unique_ptr<Schema> m_schema;
        sqlite3* m_db;
        std::map<std::string, std::unique_ptr<Collection>> m_collections;
    public:
        Database(const std::filesystem::path& path, std::unique_ptr<Schema> schema);

        // Initialize the database
        void init();

        // Get collection of the database
        Collection* getCollection(const std::string& name);
    };

    // Collection stores a list of objects
    class Collection
    {
        Schema::Collection* m_schemaCollection;
        Database* m_database;
    public:
        class Iterator {
            Collection* m_collection;
        public:
            Iterator(Collection* collection);

            // Check if the next object is available
            bool hasNext() const;

            // Get next object
            boost::json::object next();
        };

        Collection(Database* database, Schema::Collection* schemaCollection);

        // Write object to the collection
        void write(boost::json::object& data);

        // Get all objects from the collection
        Iterator getAll();
    };
};