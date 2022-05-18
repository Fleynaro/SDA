#include "Core/DataType/DataType.h"

using namespace sda;

DataType::DataType(Context* context, ObjectId* id, const std::string& name)
    : ContextObject(context, id, name)
{
    m_context->getDataTypes()->add(std::unique_ptr<DataType>(this));
}

void DataType::serialize(boost::json::object& data) const {
    ContextObject::serialize(data);
}

void DataType::deserialize(boost::json::object& data) {
    ContextObject::deserialize(data);
}

void DataType::destroy() {
    m_context->getDataTypes()->remove(getId());
}