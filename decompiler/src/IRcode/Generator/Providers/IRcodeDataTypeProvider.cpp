#include "Decompiler/IRcode/Generator/Providers/IRcodeDataTypeProvider.h"
#include "Core/DataType/ScalarDataType.h"

using namespace sda;
using namespace sda::decompiler;

IRcodeDataTypeProvider::IRcodeDataTypeProvider(Context* context)
    : m_context(context)
{}

DataType* IRcodeDataTypeProvider::getDataType(std::shared_ptr<ircode::Value> value) {
    return m_context->getDataTypes()->getScalar(ScalarType::UnsignedInt, value->getSize());
}

IRcodeSemanticsDataTypeProvider::IRcodeSemanticsDataTypeProvider(SemanticsManager* semManager)
    : IRcodeDataTypeProvider(semManager->getContext())
    , m_semManager(semManager)
{}

DataType* IRcodeSemanticsDataTypeProvider::getDataType(std::shared_ptr<ircode::Value> value) {
    return IRcodeDataTypeProvider::getDataType(value);
}