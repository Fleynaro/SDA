#include "Factory.h"
#include "Core/Context.h"
#include "Core/Function.h"

using namespace sda;

ISerializable* IFactory::create(boost::json::object& data) {
    // uuid
    if (data.find("uuid") == data.end())
        throw std::runtime_error("Object without uuid");
    std::string uuid(data["uuid"].get_string());
    ObjectId id = boost::uuids::string_generator()(uuid);

    // collection name
    std::string collection(data["collection"].get_string());

    // object type
    ObjectType type = 0;
    if (data.find("type") != data.end()) {
        type = static_cast<ObjectType>(data["type"].get_int64());
    }

    return create(&id, collection, type);
}

Factory::Factory(Context* context)
    : m_context(context)
{}

ISerializable* Factory::create(ObjectId* id, const std::string& collection, ObjectType type) {
    if (collection == "functions") {
        return new Function(m_context, id);
    }

    for (auto& factory : m_factories) {
        if (auto object = factory->create(id, collection, type))
            return object;
    }

    throw std::runtime_error("No object can be created");
}

void Factory::add(std::unique_ptr<IFactory> factory) {
    m_factories.push_back(std::move(factory));
}