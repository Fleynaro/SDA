#pragma once
#include "Symbol.h"
#include "Core/Offset.h"

namespace sda
{
    class FunctionSymbol : public Symbol
    {
        Offset m_offset;
    public:
        static inline const std::string Type = "function";

        FunctionSymbol(
            Context* context,
            Object::Id* id = nullptr,
            const std::string& name = "",
            DataType* dataType = nullptr,
            Offset offset = 0);

        Offset getOffset() const;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};