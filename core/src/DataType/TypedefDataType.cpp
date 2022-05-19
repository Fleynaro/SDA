#include "Core/DataType/TypedefDataType.h"

using namespace sda;

TypedefDataType::TypedefDataType(
    Context* context,
    Object::Id* id,
    const std::string& name,
    DataType* pointedType)
    : DataType(context, id, name)
    , m_pointedType(pointedType)
{}

size_t TypedefDataType::getSize() const {
    return m_pointedType->getSize();
}

void TypedefDataType::serialize(boost::json::object& data) const {
    DataType::serialize(data);
    data["type"] = Type;
    data["pointed_type"] = m_pointedType->serializeId();
}

void TypedefDataType::deserialize(boost::json::object& data) {
    DataType::deserialize(data);
    m_pointedType = m_context->getDataTypes()->get(data["pointed_type"]);
}