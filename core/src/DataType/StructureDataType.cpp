#include "Core/DataType/StructureDataType.h"

using namespace sda;

StructureDataType::StructureDataType(Context* context, ObjectId* id, const std::string& name)
    : DataType(context, id, name)
{}

void StructureDataType::serialize(boost::json::object& data) const {
    DataType::serialize(data);
    data["type"] = StructureDataType::Type;
}

void StructureDataType::deserialize(boost::json::object& data) {
    DataType::deserialize(data);
}