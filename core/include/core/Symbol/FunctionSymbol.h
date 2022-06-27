#pragma once
#include "Symbol.h"
#include "Core/Offset.h"

namespace sda
{
    class SymbolTable;

    class FunctionSymbol : public Symbol
    {
        SymbolTable* m_stackSymbolTable = nullptr;
        SymbolTable* m_instructionSymbolTable = nullptr;
    public:
        static inline const std::string Type = "function";

        FunctionSymbol(
            Context* context,
            Object::Id* id = nullptr,
            const std::string& name = "",
            DataType* dataType = nullptr,
            bool stackSymbolTable = false,
            bool instructionSymbolTable = false);

        SymbolTable* getStackSymbolTable() const;

        SymbolTable* getInstructionSymbolTable() const;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};