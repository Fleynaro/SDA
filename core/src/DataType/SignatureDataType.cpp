#include "Core/DataType/SignatureDataType.h"

using namespace sda;

SignatureDataType::SignatureDataType(Context* context, ObjectId* id, const std::string& name)
    : DataType(context, id, name)
{}

void SignatureDataType::serialize(boost::json::object& data) const {
    DataType::serialize(data);
    data["type"] = SignatureDataType::Type;
}

void SignatureDataType::deserialize(boost::json::object& data) {
    DataType::deserialize(data);
}