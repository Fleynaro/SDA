#include "Decompiler/Semantics/SemanticsProvider.h"

using namespace sda;
using namespace sda::decompiler;

IRcodeSemanticsDataTypeProvider::IRcodeSemanticsDataTypeProvider(SemanticsManager* semManager)
    : ircode::DataTypeProvider(semManager->getContext())
    , m_semManager(semManager)
{}

DataType* IRcodeSemanticsDataTypeProvider::getDataType(std::shared_ptr<ircode::Value> value) {
    if (auto var = std::dynamic_pointer_cast<ircode::Variable>(value)) {
        auto varId = VariableSemObj::GetId(var.get());
        if (auto varObj = m_semManager->getObject<VariableSemObj>(varId)) {
            auto semantics = varObj->findSemantics(DataTypeSemantics::FilterAll());
            if (!semantics.empty()) {
                if (auto dataTypeSem = dynamic_cast<DataTypeSemantics*>(semantics.front()))
                    return dataTypeSem->getDataType();
            }
        }
    }
    return ircode::DataTypeProvider::getDataType(value);
}