#include "Factory.h"
#include "Core/Context.h"
#include "Core/Function.h"

using namespace sda;

ISerializable* IFactory::create(IContext* context, boost::json::object& data) {
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

    return create(context, &id, collection, type);
}

ISerializable* Factory::create(IContext* context, ObjectId* id, const std::string& collection, ObjectType type) {
    if (auto ctx = dynamic_cast<Context*>(context)) {
        if (collection == "functions") {
            return new Function(ctx, id);
        }
    }

    for (auto& factory : m_factories) {
        if (auto object = factory->create(context, id, collection, type))
            return object;
    }

    throw std::runtime_error("No object can be created");
}

void Factory::add(std::unique_ptr<IFactory> factory) {
    m_factories.push_back(std::move(factory));
}