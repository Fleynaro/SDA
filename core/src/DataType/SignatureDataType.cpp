#include "Core/DataType/SignatureDataType.h"
#include "Core/Context.h"

using namespace sda;

SignatureDataType::SignatureDataType(
    Context* context,
    std::shared_ptr<CallingConvention> callingConvention,
    Object::Id* id,
    const std::string& name)
    : DataType(context, id, name)
    , m_callingConvention(callingConvention)
{
    m_context->getDataTypes()->add(std::unique_ptr<SignatureDataType>(this));
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

void SignatureDataType::setParameters(const std::vector<FunctionParameterSymbol*>& parameters) {
    m_context->getCallbacks()->onObjectModified(this);
    m_parameters = parameters;
    m_updateStorages = true;
}

const std::vector<FunctionParameterSymbol*>& SignatureDataType::getParameters() const {
    return m_parameters;
}

void SignatureDataType::setReturnType(DataType* returnType) {
    m_context->getCallbacks()->onObjectModified(this);
    m_returnType = returnType;
    m_updateStorages = true;
}

DataType* SignatureDataType::getReturnType() const {
    return m_returnType;
}

size_t SignatureDataType::getSize() const {
    return 0;
}

void SignatureDataType::serialize(boost::json::object& data) const {
    DataType::serialize(data);
    data["type"] = Type;

    if(auto serCallingConvention = std::dynamic_pointer_cast<ISerializable>(m_callingConvention)) {
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

    if(auto serCallingConvention = std::dynamic_pointer_cast<ISerializable>(m_callingConvention)) {
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
}