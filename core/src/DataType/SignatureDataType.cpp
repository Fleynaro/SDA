#include "SDA/Core/DataType/SignatureDataType.h"
#include "SDA/Core/Symbol/FunctionSymbol.h"
#include "SDA/Core/Context.h"

using namespace sda;

SignatureDataType::SignatureDataType(
    Context* context,
    std::shared_ptr<CallingConvention> callingConvention,
    Object::Id* id,
    const std::string& name,
    DataType* returnType,
    const std::vector<FunctionParameterSymbol*>& parameters)
    : DataType(context, id, name)
    , m_callingConvention(callingConvention)
    , m_returnType(returnType)
    , m_parameters(parameters)
{
    if (!m_returnType)
        m_returnType = m_context->getDataTypes()->getByName("void");
    m_context->getDataTypes()->add(std::unique_ptr<SignatureDataType>(this));
}

std::list<FunctionSymbol*> SignatureDataType::getFunctionSymbols() const {
    std::list<FunctionSymbol*> result;
    for (auto parent : getParents()) {
        if (auto funcSymbol = dynamic_cast<FunctionSymbol*>(parent)) {
            result.push_back(funcSymbol);
        }
    }
    return result;
}

std::shared_ptr<CallingConvention> SignatureDataType::getCallingConvention() const {
    return m_callingConvention;
}

const CallingConvention::Map& SignatureDataType::getStorages() {
    if (m_updateStorages) {
        m_storages = m_callingConvention->getStorages(this);
        m_updateStorages = false;
    }
    return m_storages;
}

const CallingConvention::StorageInfo* SignatureDataType::findStorageInfo(const CallingConvention::Storage& storage) {
    auto& storages = getStorages();
    auto it = storages.find(storage);
    if (it != storages.end())
        return &it->second;
    return nullptr;
}

void SignatureDataType::setParameters(const std::vector<FunctionParameterSymbol*>& parameters) {
    notifyModified(Object::ModState::Before);
    m_parameters = parameters;
    m_updateStorages = true;
    notifyModified(Object::ModState::After);
}

const std::vector<FunctionParameterSymbol*>& SignatureDataType::getParameters() const {
    return m_parameters;
}

void SignatureDataType::setReturnType(DataType* returnType) {
    notifyModified(Object::ModState::Before);
    m_returnType = returnType;
    m_updateStorages = true;
    notifyModified(Object::ModState::After);
}

DataType* SignatureDataType::getReturnType() const {
    return m_returnType;
}

size_t SignatureDataType::getSize() const {
    return 1;
}

void SignatureDataType::clear() {
    for (auto param : m_parameters) {
        param->destroy();
    }
    setParameters({});
    setReturnType(m_context->getDataTypes()->getByName("void"));
}

void SignatureDataType::serialize(boost::json::object& data) const {
    DataType::serialize(data);
    data["type"] = Type;

    if(auto serCallingConvention = std::dynamic_pointer_cast<utils::ISerializable>(m_callingConvention)) {
        boost::json::object callingConventionData;
        serCallingConvention->serialize(callingConventionData);
        data["calling_convention"] = callingConventionData;
    }

    // serialize the list of parameters
    auto parametersIds = boost::json::array();
    for (auto parameter : m_parameters)
        parametersIds.push_back(parameter->serializeId());
    data["parameters"] = parametersIds;

    data["return_type"] = m_returnType->serializeId();
}

void SignatureDataType::deserialize(boost::json::object& data) {
    DataType::deserialize(data);

    if(auto serCallingConvention = std::dynamic_pointer_cast<utils::ISerializable>(m_callingConvention)) {
        serCallingConvention->deserialize(data["calling_convention"].get_object());
    }

    // deserialize the list of parameters
    m_parameters.clear();
    const auto& parametersIds = data["parameters"].get_array();
    for (const auto& parameterId : parametersIds) {
        auto symbol = m_context->getSymbols()->get(parameterId);
        if (auto parameter = dynamic_cast<FunctionParameterSymbol*>(symbol))
            m_parameters.push_back(parameter);
    }

    m_returnType = m_context->getDataTypes()->get(data["return_type"]);
    m_updateStorages = true;
    notifyModified(Object::ModState::After);
}