#pragma once
#include "SDA/Core/IRcode/IRcodeDataTypeProvider.h"
#include "SDA/Decompiler/Semantics/SemanticsManager.h"

namespace sda::decompiler
{
    class IRcodeSemanticsDataTypeProvider : public ircode::DataTypeProvider
    {
        SemanticsManager* m_semManager;
    public:
        IRcodeSemanticsDataTypeProvider(SemanticsManager* semManager);

        DataType* getDataType(std::shared_ptr<ircode::Value> value) override;
    };
};