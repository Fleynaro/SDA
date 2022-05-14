#include "Factory.h"
#include "Core/Function.h"

using namespace sda;

IObject* Factory::create(Context* context, ObjectId* id, const std::string& collection, ObjectType type) {
    if (collection == "functions") {
        return new Function(context, id);
    }

    for (auto& factory : m_factories) {
        if (auto object = factory->create(context, id, collection, type))
            return object;
    }
}

void Factory::add(std::unique_ptr<IFactory> factory) {
    m_factories.push_back(std::move(factory));
}