#include "Factory.h"
#include "Core/Context.h"
#include "Core/Image/AddressSpace.h"
#include "Core/Image/Image.h"
#include "Core/Image/ImageContext.h"
#include "Core/DataType/PointerDataType.h"
#include "Core/DataType/ArrayDataType.h"
#include "Core/DataType/TypedefDataType.h"
#include "Core/DataType/ScalarDataType.h"
#include "Core/DataType/EnumDataType.h"
#include "Core/DataType/SignatureDataType.h"
#include "Core/DataType/StructureDataType.h"
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
    if (collection == AddressSpace::Collection) {
        return new AddressSpace(m_context, id);
    } else if (collection == Image::Collection) {
        if(data.find("reader") != data.end() && data.find("analyser") != data.end()) {  
            std::string readerType(data["reader"].get_object()["type"].get_string());
            std::unique_ptr<IImageReader> reader;
            if (readerType == FileImageReader::Name)
                reader = std::make_unique<FileImageReader>();
            
            std::string analyserType(data["analyser"].get_object()["type"].get_string());
            std::shared_ptr<IImageAnalyser> analyser;
            if (analyserType == PEImageAnalyser::Name)
                analyser = std::make_shared<PEImageAnalyser>();
  
            return new Image(m_context, std::move(reader), analyser, id);
        }
    } else if (collection == ImageContext::Collection) {
        return new ImageContext(m_context, id);
    } else if (collection == DataType::Collection) {
        if (data.find("type") != data.end()) {
            std::string type(data["type"].get_string());
            if (type == PointerDataType::Type)
                return new PointerDataType(m_context, id);
            else if (type == ArrayDataType::Type)
                return new ArrayDataType(m_context, id);
            else if (type == TypedefDataType::Type)
                return new TypedefDataType(m_context, id);
            else if (type == ScalarDataType::Type)
                return new ScalarDataType(m_context, id);
            else if (type == EnumDataType::Type)
                return new EnumDataType(m_context, id);
            else if (type == SignatureDataType::Type)
                return new SignatureDataType(m_context, id);
            else if (type == StructureDataType::Type)
                return new StructureDataType(m_context, id);
        }
    } else if (collection == Symbol::Collection) {
        return new Symbol(m_context, id);
    } else if (collection == SymbolTable::Collection) {
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