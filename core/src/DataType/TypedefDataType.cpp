#include "SDA/Core/DataType/TypedefDataType.h"

using namespace sda;

TypedefDataType::TypedefDataType(
    Context* context,
    Object::Id* id,
    const std::string& name,
    DataType* refType)
    : DataType(context, id, name)
    , m_refType(refType)
{
    m_context->getDataTypes()->add(std::unique_ptr<TypedefDataType>(this));
}

void TypedefDataType::setReferenceType(DataType* refType) {
    if (m_refType == refType)
        return;
    notifyModified(Object::ModState::Before);
    m_refType = refType;
    notifyModified(Object::ModState::After);
}

DataType* TypedefDataType::getReferenceType() const {
    return m_refType;
}

DataType* TypedefDataType::getBaseType() {
    return m_refType->getBaseType();
}

size_t TypedefDataType::getSize() const {
    return m_refType->getSize();
}

void TypedefDataType::serialize(boost::json::object& data) const {
    DataType::serialize(data);
    data["type"] = Type;
    data["ref_type"] = m_refType->serializeId();
}

void TypedefDataType::deserialize(boost::json::object& data) {
    DataType::deserialize(data);
    m_refType = m_context->getDataTypes()->get(data["ref_type"]);
    notifyModified(Object::ModState::After);
}