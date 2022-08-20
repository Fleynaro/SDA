#pragma once
#include "Decompiler/Semantics/SemanticsManager.h"

namespace sda::decompiler
{
    class IRcodeDataTypeProvider
    {
        Context* m_context;
    public:
        IRcodeDataTypeProvider(Context* context);

        virtual DataType* getDataType(std::shared_ptr<ircode::Value> value);
    };

    class IRcodeSemanticsDataTypeProvider : public IRcodeDataTypeProvider
    {
        SemanticsManager* m_semManager;
    public:
        IRcodeSemanticsDataTypeProvider(SemanticsManager* semManager);

        DataType* getDataType(std::shared_ptr<ircode::Value> value) override;
    };
};