#include "Core/DataType/PointerDataType.h"

using namespace sda;

PointerDataType::PointerDataType(
    Context* context,
    Object::Id* id,
    DataType* pointedType)
    : DataType(context, id),
    m_pointedType(pointedType)
{
    if (pointedType) {
        setName(m_pointedType->getName() + "*");
    }
}

DataType* PointerDataType::getPointedType() const {
    return m_pointedType;
}

size_t PointerDataType::getSize() const {
    return 0x8;
}

void PointerDataType::serialize(boost::json::object& data) const {
    DataType::serialize(data);
    data["type"] = Type;
    data["pointed_type"] = m_pointedType->serializeId();
}

void PointerDataType::deserialize(boost::json::object& data) {
    DataType::deserialize(data);
    m_pointedType = m_context->getDataTypes()->get(data["pointed_type"]);
}