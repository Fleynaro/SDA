#include "Core/DataType/PointerDataType.h"

using namespace sda;

PointerDataType::PointerDataType(
    Context* context,
    ObjectId* id,
    DataType* pointedType)
    : DataType(context, id),
    m_pointedType(pointedType)
{
    if (pointedType) {
        createName();
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
    data["type"] = PointerDataType::Type;
    data["pointed_type"] = m_pointedType->serializeId();
}

void PointerDataType::deserialize(boost::json::object& data) {
    DataType::deserialize(data);
    m_pointedType = m_context->getDataTypes()->get(data["pointed_type"]);
    createName();
}

void PointerDataType::createName() {
    setName(m_pointedType->getName() + "*");
}