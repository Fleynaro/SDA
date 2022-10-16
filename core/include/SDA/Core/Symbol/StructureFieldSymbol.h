#pragma once
#include "Symbol.h"
#include "SDA/Core/Offset.h"

namespace sda
{
    class StructureFieldSymbol : public Symbol
    {
    public:
        static inline const std::string Type = "structure_field";

        StructureFieldSymbol(
            Context* context,
            Object::Id* id = nullptr,
            const std::string& name = "",
            DataType* dataType = nullptr);

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};