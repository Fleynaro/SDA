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
    // Open database
    if (sqlite3_open(m_path.string().c_str(), &m_db) != SQLITE_OK) {
        throw std::runtime_error(std::string("Can't open database: ") + sqlite3_errmsg(m_db));
    }

    // Create tables if they don't exist and collections
    for(const auto& collection : m_schema->getCollections()) {
        createTableIfNotExists(collection);
        m_collections[collection.name] = std::make_unique<Collection>(this, &collection);
    }
}

Collection* Database::getCollection(const std::string& name) {
    auto it = m_collections.find(name);
    if(it == m_collections.end()) {
        throw std::runtime_error(std::string("Collection not found: ") + name);
    }
    return it->second.get();
}

Schema* Database::getSchema() {
    return m_schema.get();
}

void Database::createTableIfNotExists(const Schema::Collection& collection) {
    std::stringstream sql;
    sql << "CREATE TABLE IF NOT EXISTS " << collection.name << " (";
    sql << "uuid TEXT PRIMARY KEY NOT NULL,";
    sql << "data TEXT NOT NULL";

    for(const auto& field : collection.fields) {
        sql << ",";
        sql << field.name + " ";

        switch(field.type) {
        case Schema::Field::INTEGER:
            sql << "INTEGER";
            break;
        case Schema::Field::REAL:
            sql << "REAL";
            break;
        case Schema::Field::TEXT:
            sql << "TEXT";
            break;
        }

        if(field.isNotNull)
            sql << " NOT NULL";
        if(field.isUniqueKey)
            sql << " UNIQUE";
    }
    
    sql << ");";

    if (sqlite3_exec(m_db, sql.str().c_str(), nullptr, nullptr, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Can't create table: ") + sqlite3_errmsg(m_db));
    }
}

Collection::Collection(Database* database, const Schema::Collection* schemaCollection)
    : m_database(database), m_schemaCollection(schemaCollection)
{}

void Collection::write(boost::json::object& data) {
    std::stringstream sql;
    sql << "REPLACE INTO " << m_schemaCollection->name << " VALUES (";
    sql << "'" << data["uuid"] << "',";
    sql << "'" << data << "'";

    for(const auto& field : m_schemaCollection->fields) {
        sql << ",";
        if (field.type == Schema::Field::TEXT) {
            sql << "'" << data[field.name] << "'";
        } else {
            sql << data[field.name];
        }
    }

    sql << ");";

    if (sqlite3_exec(m_database->m_db, sql.str().c_str(), nullptr, nullptr, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Can't write data: ") + sqlite3_errmsg(m_database->m_db));
    }
}

void Collection::remove(boost::json::object& data) {
    std::stringstream sql;
    sql << "DELETE FROM " << m_schemaCollection->name << " WHERE uuid = '" << data["uuid"] << "';";

    if (sqlite3_exec(m_database->m_db, sql.str().c_str(), nullptr, nullptr, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Can't remove data: ") + sqlite3_errmsg(m_database->m_db));
    }
}

std::unique_ptr<Collection::Iterator> Collection::getAll() {
    return query("");
}

std::unique_ptr<Collection::Iterator> Collection::query(const std::string& suffix) {
    std::stringstream sql;
    sql << "SELECT data FROM " << m_schemaCollection->name << suffix;

    sqlite3_stmt* stmt;
    if (sqlite3_prepare(m_database->m_db, sql.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Can't get data: ") + sqlite3_errmsg(m_database->m_db));
    }
    return std::make_unique<Iterator>(stmt);
}

Collection::Iterator::Iterator(sqlite3_stmt* stmt)
    : m_stmt(stmt)
{}

Collection::Iterator::~Iterator() {
    sqlite3_finalize(m_stmt);
}

bool Collection::Iterator::hasNext() const {
    return sqlite3_step(m_stmt) == SQLITE_ROW;
}

boost::json::object Collection::Iterator::next() {
    std::string data = (const char*)sqlite3_column_text(m_stmt, 0);
    return boost::json::parse(data).as_object();
}