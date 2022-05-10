#include "Database/Database.h"

using namespace sda;

Schema::Schema(const std::list<Collection>& collections)
    : m_collections(collections)
{}

const std::list<Schema::Collection>& Schema::getCollections() const {
    return m_collections;
}

Database::Database(const std::filesystem::path& path, std::unique_ptr<Schema> schema)
    : m_path(path), m_schema(std::move(schema))
{}

void Database::init() {
    // Create database file if it doesn't exist
    if(!std::filesystem::exists(m_path)) {
        
    }

    if (sqlite3_open(m_path.string().c_str(), &m_db)) {
        throw std::runtime_error(std::string("Can't open database: ") + sqlite3_errmsg(m_db));
    }

    // Create collections
    for(const auto& collection : m_schema->getCollections()) {
        m_collections[collection.name] = std::make_unique<Collection>(this, &collection);
    }
}

Collection* Database::getCollection(const std::string& name) {
    return m_collections[name].get();
}