#pragma once
#include "Symbol.h"
#include "Core/Offset.h"

namespace sda
{
    class RegisterVariableSymbol : public Symbol
    {
        std::list<ComplexOffset> m_offsets;
    public:
        static inline const std::string Type = "register_variable";

        RegisterVariableSymbol(
            Context* context,
            Object::Id* id = nullptr,
            const std::string& name = "",
            DataType* dataType = nullptr,
            const std::list<ComplexOffset>& offsets = std::list<ComplexOffset>());

        const std::list<ComplexOffset>& getOffsets() const;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};