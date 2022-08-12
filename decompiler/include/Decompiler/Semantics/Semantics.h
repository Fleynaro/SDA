#pragma once
#include "Core/SymbolTable/SymbolTable.h"

namespace sda::decompiler
{
    class Semantics
    {
        std::list<Semantics*> m_nextSemantics;
        std::list<Semantics*> m_prevSemantics;
    public:
        using FilterFunction = std::function<bool(const Semantics*)>;

        virtual const std::string& getName() const = 0;
    };

    class DataTypeSemantics : public Semantics
    {
        DataType* m_dataType;
        SymbolTable* m_symbolTable;
    public:
        DataTypeSemantics(DataType* dataType, SymbolTable* symbolTable);

        const std::string& getName() const override;

        DataType* getDataType() const;

        SymbolTable* getSymbolTable() const;

        using DataTypeFilterFunction = std::function<bool(const DataTypeSemantics*)>;

        static FilterFunction Filter(const DataTypeFilterFunction& filter = [](const DataTypeSemantics*){ return true; });
    };
};