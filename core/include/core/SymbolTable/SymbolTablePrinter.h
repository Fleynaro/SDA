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

        static std::string Print(SymbolTable* symbolTable, Context* context, bool withName = false);

        void printDef(SymbolTable* symbolTable, bool withName = false);

    protected:
        virtual void printDataType(DataType* dataType);

        void printTokenImpl(const std::string& text, Token token) const override;
    };
};