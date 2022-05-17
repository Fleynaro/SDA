#include "Factory.h"
#include "Core/Context.h"
#include "Core/Image/AddressSpace.h"
#include "Core/Image/Image.h"
#include "Core/DataType/DataType.h"
#include "Core/Symbol/Symbol.h"
#include "Core/Symbol/SymbolTable.h"

using namespace sda;

ISerializable* IFactory::create(boost::json::object& data) {
    // uuid
    if (data.find("uuid") == data.end())
        throw std::runtime_error("Object without uuid");
    std::string uuid(data["uuid"].get_string());
    auto id = boost::uuids::string_generator()(uuid);

    // collection name
    std::string collection(data["collection"].get_string());

    return create(&id, collection, data);
}

Factory::Factory(Context* context)
    : m_context(context)
{}

ISerializable* Factory::create(boost::uuids::uuid* id, const std::string& collection, boost::json::object& data) {
    if (collection == AddressSpace::GetCollectionName()) {
        return new AddressSpace(m_context, id);
    } else if (collection == Image::GetCollectionName()) {
        if(data.find("reader") != data.end() && data.find("analyser") != data.end()) {   
            std::string readerStr(data["reader"].get_string());
            std::unique_ptr<IImageReader> reader;
            
            std::string analyserStr(data["analyser"].get_string());
            std::shared_ptr<IImageAnalyser> analyser;

            auto image = new Image(m_context, std::move(reader), analyser, id);
            image->analyse();
            return image;
        }
    } else if (collection == DataType::GetCollectionName()) {
        return new DataType(m_context, id);
    } else if (collection == Symbol::GetCollectionName()) {
        return new Symbol(m_context, id);
    } else if (collection == SymbolTable::GetCollectionName()) {
        return new SymbolTable(m_context, id);
    }

    for (auto& factory : m_factories) {
        if (auto object = factory->create(id, collection, data))
            return object;
    }

    throw std::runtime_error("Object cannot be created");
}

void Factory::add(std::unique_ptr<IFactory> factory) {
    m_factories.push_back(std::move(factory));
}