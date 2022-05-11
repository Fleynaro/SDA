#include "Database/Loader.h"
#include "Core/Context.h"
#include "Core/Function.h"

using namespace sda;

Loader::Loader(Database* database, Context* context)
    : m_database(database), m_context(context)
{}

void Loader::load() {
    loadCollection("functions", [&]() { return new Function(m_context); });

    for (auto& [object, data] : m_objects) {
        object->deserialize(data);
    }
}

void Loader::loadCollection(const std::string& name, std::function<ISerializable*()> creator) {
    auto it = m_database->getCollection(name)->getAll();
    while (it->hasNext()) {
        m_objects.push_back(std::make_pair(creator(), it->next()));
    }
}