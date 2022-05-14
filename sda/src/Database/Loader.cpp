#include "Database/Loader.h"
#include <boost/uuid/uuid_generators.hpp>
#include "Core/Context.h"
#include "Core/Function.h"
#include "Factory.h"

using namespace sda;

Loader::Loader(Database* database, Context* context, IFactory* factory)
    : m_database(database), m_context(context), m_factory(factory)
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

        if (data.find("uuid") == data.end())
            throw std::runtime_error("Object without uuid");
        std::string uuid(data["uuid"].get_string());
        ObjectId id = boost::uuids::string_generator()(uuid);

        ObjectType type = 0;
        if (data.find("type") != data.end()) {
            type = static_cast<ObjectType>(data["type"].get_int64());
        }

        auto object = dynamic_cast<ISerializable*>(m_factory->create(m_context, &id, name, type));
        if (!object)
            throw std::runtime_error("Object is not serializable or has not created by a factory");
        m_objects.push_back(std::make_pair(object, data));
    }
}