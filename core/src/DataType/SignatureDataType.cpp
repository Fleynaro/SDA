#include "Core/DataType/SignatureDataType.h"

using namespace sda;

SignatureDataType::SignatureDataType(Context* context, Object::Id* id, const std::string& name)
    : DataType(context, id, name)
{}

void SignatureDataType::serialize(boost::json::object& data) const {
    DataType::serialize(data);
    data["type"] = Type;
}

void SignatureDataType::deserialize(boost::json::object& data) {
    DataType::deserialize(data);
}