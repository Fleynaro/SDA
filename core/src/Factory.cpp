#include "SDA/Core/Factory.h"
#include "SDA/Core/Image/AddressSpace.h"
#include "SDA/Core/Image/Image.h"
#include "SDA/Core/DataType/VoidDataType.h"
#include "SDA/Core/DataType/PointerDataType.h"
#include "SDA/Core/DataType/ArrayDataType.h"
#include "SDA/Core/DataType/TypedefDataType.h"
#include "SDA/Core/DataType/ScalarDataType.h"
#include "SDA/Core/DataType/EnumDataType.h"
#include "SDA/Core/DataType/SignatureDataType.h"
#include "SDA/Core/DataType/StructureDataType.h"
#include "SDA/Core/Symbol/VariableSymbol.h"
#include "SDA/Core/Symbol/FunctionSymbol.h"
#include "SDA/Core/Symbol/FunctionParameterSymbol.h"
#include "SDA/Core/Symbol/StructureFieldSymbol.h"
#include "SDA/Core/SymbolTable/StandartSymbolTable.h"
#include "SDA/Core/SymbolTable/OptimizedSymbolTable.h"

using namespace sda;

Factory::Factory(Context* context)
    : m_context(context)
{}

ContextObject* Factory::create(boost::uuids::uuid* id, const std::string& className, boost::json::object& data) {
    if (className == AddressSpace::Class) {
        return new AddressSpace(m_context, id);
    } else if (className == Image::Class) {
        if(data.find("rw") == data.end() || !data["rw"].is_object())
            throw std::runtime_error("Image without rw");
        auto rwObj = data["rw"].get_object();
        if (rwObj.find("type") == rwObj.end() || !rwObj["type"].is_string())
            throw std::runtime_error("Image without rw type");

        std::string rwType(rwObj["type"].get_string().c_str());
        std::unique_ptr<IImageRW> rw;
        if (rwType == FileImageRW::Name)
            rw = std::make_unique<FileImageRW>();

        if (rw) {
            if (data.find("analyser") == data.end() || !data["analyser"].is_object())
                throw std::runtime_error("Image without analyser");
            auto analyserObj = data["analyser"].get_object();
            if (analyserObj.find("type") == analyserObj.end() || !analyserObj["type"].is_string())
                throw std::runtime_error("Image without analyser type");
            std::string analyserType(analyserObj["type"].get_string().c_str());
            std::shared_ptr<ImageAnalyser> analyser;
            if (analyserType == PEImageAnalyser::Name)
                analyser = std::make_shared<PEImageAnalyser>();

            if (analyser) {
                return new Image(m_context, std::move(rw), analyser, id);
            }
        }
    } else if (className == DataType::Class) {
        if (data.find("type") == data.end() || !data["type"].is_string())
            throw std::runtime_error("DataType without type");
        std::string type(data["type"].get_string().c_str());
        if (type == VoidDataType::Type)
            return new VoidDataType(m_context, id);
        else if (type == PointerDataType::Type)
            return new PointerDataType(m_context, id);
        else if (type == ArrayDataType::Type)
            return new ArrayDataType(m_context, id);
        else if (type == TypedefDataType::Type)
            return new TypedefDataType(m_context, id);
        else if (type == ScalarDataType::Type)
            return new ScalarDataType(m_context, id);
        else if (type == EnumDataType::Type)
            return new EnumDataType(m_context, id);
        else if (type == StructureDataType::Type)
            return new StructureDataType(m_context, id);
        else if (type == SignatureDataType::Type) {
            if(data.find("calling_convention") == data.end())
                throw std::runtime_error("SignatureDataType without callingConvention");
            auto callingConventionObj = data["calling_convention"].get_object();
            if (callingConventionObj.find("type") == callingConventionObj.end() || !callingConventionObj["type"].is_string())
                throw std::runtime_error("SignatureDataType without callingConvention type");
            std::string ccType(callingConventionObj["type"].get_string().c_str());
            std::shared_ptr<CallingConvention> callingConvention;
            if (ccType == CustomCallingConvention::Name)
                callingConvention = std::make_shared<CustomCallingConvention>();
            else {
                for (auto& cc : m_context->getPlatform()->getCallingConventions()) {
                    if (ccType == cc->getName()) {
                        callingConvention = cc;
                        break;
                    }
                }
            }

            if (callingConvention) {
                return new SignatureDataType(m_context, callingConvention, id);
            }
        }
    } else if (className == Symbol::Class) {
        if (data.find("type") == data.end() || !data["type"].is_string())
            throw std::runtime_error("Symbol without type");
        std::string type(data["type"].get_string().c_str());
        if (type == VariableSymbol::Type)
            return new VariableSymbol(m_context, id);
        else if (type == FunctionSymbol::Type)
            return new FunctionSymbol(m_context, id);
        else if (type == FunctionParameterSymbol::Type)
            return new FunctionParameterSymbol(m_context, id);
        else if (type == StructureFieldSymbol::Type)
            return new StructureFieldSymbol(m_context, id);
    } else if (className == SymbolTable::Class) {
        if (data.find("type") == data.end() || !data["type"].is_string())
            throw std::runtime_error("SymbolTable without type");
        std::string type(data["type"].get_string().c_str());
        if (type == StandartSymbolTable::Type)
            return new StandartSymbolTable(m_context, id);
        else if (type == OptimizedSymbolTable::Type)
            return new OptimizedSymbolTable(m_context, id);
    }

    throw std::runtime_error("Object cannot be created");
}

ContextObject* Factory::create(boost::json::object& data) {
    // uuid
    if (data.find("uuid") == data.end() || !data["uuid"].is_string())
        throw std::runtime_error("Object without uuid");
    std::string uuid(data["uuid"].get_string().c_str());
    auto id = boost::uuids::string_generator()(uuid);

    // class name
    if (data.find("class") == data.end() || !data["class"].is_string())
        throw std::runtime_error("Object without class");
    std::string className(data["class"].get_string().c_str());

    return create(&id, className, data);
}
