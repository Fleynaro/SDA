#pragma once
#include "IRcodeOperation.h"

namespace sda::ircode
{
    class DataTypeProvider
    {
        Context* m_context;
    public:
        DataTypeProvider(Context* context);

        virtual DataType* getDataType(std::shared_ptr<ircode::Value> value);
    };
};