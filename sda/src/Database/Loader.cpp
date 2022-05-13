#include "Database/Loader.h"
#include "Core/Context.h"
#include "Core/Function.h"

using namespace sda;

Loader::Loader(Database* database, Context* context)
    : m_database(database), m_context(context)
{}

void Loader::load() {
    loadCollection("functions", [&](int type) { return new Function(m_context); });

    for (auto& [object, data] : m_objects) {
        object->deserialize(data);
    }
}

void Loader::loadCollection(const std::string& name, std::function<ISerializable*(int)> creator) {
    auto it = m_database->getCollection(name)->getAll();
    while (it->hasNext()) {
        auto data = it->next();
        int type = 0;
        if (data.find("type") != data.end()) {
            type = static_cast<int>(data["type"].get_int64());
        }
        m_objects.push_back(std::make_pair(creator(type), data));
    }
}