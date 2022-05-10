#pragma once
#include <list>
#include <map>
#include <filesystem>
#include <Core/Serialization.h>

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
            std::vector<Field> fields;
        };

        Schema() = default;

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
        std::map<std::string, std::unique_ptr<Collection>> m_collections;
    public:
        Database(const std::filesystem::path& path, std::unique_ptr<Schema> schema);

        // Initialize the database
        void init();

        // Get path to the database
        const std::filesystem::path& getPath() const;

        // Get schema of the database
        Schema* getSchema();

        // Get collection of the database
        Collection* getCollection(const std::string& name);
    };

    // Collection stores a list of objects
    class Collection
    {
        std::string m_name;
        Database* m_database;
    public:
        Collection(const std::string& name, Database* database);

        // Write object to the collection
        void write(ISerializable* object);

        // Get all objects from the collection
        std::list<ISerializable*> getAll();
    };
};