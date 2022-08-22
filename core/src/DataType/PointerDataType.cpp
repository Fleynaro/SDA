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
        setName(GetTypeName(pointedType));
    }
    m_context->getDataTypes()->add(std::unique_ptr<PointerDataType>(this));
}

DataType* PointerDataType::getPointedType() const {
    return m_pointedType;
}

bool PointerDataType::isPointer() const {
    return true;
}

size_t PointerDataType::getSize() const {
    return m_context->getPlatform()->getPointerSize();
}

void PointerDataType::serialize(boost::json::object& data) const {
    DataType::serialize(data);
    data["type"] = Type;
    data["pointed_type"] = m_pointedType->serializeId();
}

void PointerDataType::deserialize(boost::json::object& data) {
    DataType::deserialize(data);
    m_pointedType = m_context->getDataTypes()->get(data["pointed_type"]);
    notifyModified(Object::ModState::After);
}

std::string PointerDataType::GetTypeName(DataType* pointedType) {
    return pointedType->getName() + "*";
}