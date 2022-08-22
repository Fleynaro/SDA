#include "Core/IRcode/IRcodeDataTypeProvider.h"
#include "Core/DataType/ScalarDataType.h"

using namespace sda;
using namespace sda::ircode;

DataTypeProvider::DataTypeProvider(Context* context)
    : m_context(context)
{}

DataType* DataTypeProvider::getDataType(std::shared_ptr<ircode::Value> value) {
    return m_context->getDataTypes()->getScalar(ScalarType::UnsignedInt, value->getSize());
}