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
        // Field is a column in a table
        struct Field {
            std::string name;
            enum {
                INTEGER,
                REAL,
                TEXT
            } type;
            bool isNotNull = false;
            bool isUniqueKey = false;
        };

        // Collection is a table in the database
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
        friend class Collection;
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

        // Get schema of the database
        Schema* getSchema();

    private:
        // Create table in the database if it doesn't exist
        void createTableIfNotExists(const Schema::Collection& collection);
    };

    // Collection stores a list of objects
    class Collection
    {
        const Schema::Collection* m_schemaCollection;
        Database* m_database;
    public:
        class Iterator {
            sqlite3_stmt* m_stmt;
        public:
            Iterator(sqlite3_stmt* stmt);

            ~Iterator();

            // Check if the next object is available
            bool hasNext() const;

            // Get next object
            boost::json::object next();
        };

        Collection(Database* database, const Schema::Collection* schemaCollection);

        // Write object to the collection
        void write(boost::json::object& data);

        // Remove object from the collection
        void remove(boost::json::object& data);

        // Get all objects from the collection
        std::unique_ptr<Iterator> getAll();

    private:
        // Query the database
        std::unique_ptr<Iterator> query(const std::string& suffix);
    };
};