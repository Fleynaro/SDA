#pragma once
#include "Symbol.h"

namespace sda
{
    class FunctionParameterSymbol : public Symbol
    {
    public:
        static inline const std::string Type = "memory_variable";

        FunctionParameterSymbol(
            Context* context,
            Object::Id* id = nullptr,
            const std::string& name = "",
            DataType* dataType = nullptr);

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};