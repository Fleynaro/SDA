#pragma once
#include "Core/SymbolTable/SymbolTable.h"
#include "Core/DataType/DataType.h"
#include "Core/Utils/AbstractPrinter.h"

namespace sda
{
    class SymbolTablePrinter : public utils::AbstractPrinter
    {
        Context* m_context;
    public:
        static inline const Token DATATYPE = PARENT;

        SymbolTablePrinter(Context* context);

        static std::string Print(SymbolTable* symbolTable, Context* context, bool withName = true);

        void printDef(SymbolTable* symbolTable, bool withName = true);

    protected:
        virtual void printDataType(DataType* dataType);

        void printTokenImpl(const std::string& text, Token token) const override;
    };
};