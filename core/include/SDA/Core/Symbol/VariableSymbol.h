#pragma once
#include "Symbol.h"
#include "SDA/Core/Offset.h"

namespace sda
{
    class VariableSymbol : public Symbol
    {
    public:
        static inline const std::string Type = "variable";

        VariableSymbol(
            Context* context,
            Object::Id* id = nullptr,
            const std::string& name = "",
            DataType* dataType = nullptr);

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};