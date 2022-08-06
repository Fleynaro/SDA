#include "Core/DataType/VoidDataType.h"

using namespace sda;

VoidDataType::VoidDataType(Context* context, Object::Id* id)
    : DataType(context, id, "void")
{
    m_context->getDataTypes()->add(std::unique_ptr<VoidDataType>(this));
}

size_t VoidDataType::getSize() const {
    return 0;
}

void VoidDataType::serialize(boost::json::object& data) const {
    DataType::serialize(data);
    data["type"] = Type;
}

void VoidDataType::deserialize(boost::json::object& data) {
    DataType::deserialize(data);
}