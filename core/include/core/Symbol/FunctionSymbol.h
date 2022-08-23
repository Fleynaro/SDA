#pragma once
#include "Symbol.h"
#include "Core/Offset.h"
#include "Core/DataType/SignatureDataType.h"

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
            SignatureDataType* dataType = nullptr,
            SymbolTable* stackSymbolTable = nullptr,
            SymbolTable* instructionSymbolTable = nullptr);

        SignatureDataType* getSignature() const;

        SymbolTable* getStackSymbolTable() const;

        SymbolTable* getInstructionSymbolTable() const;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};