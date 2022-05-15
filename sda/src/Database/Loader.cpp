#include "Database/Loader.h"
#include <boost/uuid/uuid_generators.hpp>
#include "Core/Context.h"
#include "Core/Function.h"
#include "Factory.h"

using namespace sda;

Loader::Loader(Database* database, IFactory* factory)
    : m_database(database), m_factory(factory)
{}

void Loader::load() {
    for (const auto& collection : m_database->getSchema()->getCollections()) {
        loadCollection(collection.name);
    }

    for (auto& [object, data] : m_objects) {
        object->deserialize(data);
    }
}

void Loader::loadCollection(const std::string& name) {
    auto it = m_database->getCollection(name)->getAll();
    while (it->hasNext()) {
        auto data = it->next();
        auto object = m_factory->create(data);
        m_objects.push_back(std::make_pair(object, data));
    }
}