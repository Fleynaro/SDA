#pragma once
#include "Core/SymbolTable/SymbolTable.h"

namespace sda::decompiler
{
    class SemanticsObject;

    class Semantics
    {
        SemanticsObject* m_sourceObject;
        std::list<Semantics*> m_nextSemantics;
        std::list<Semantics*> m_prevSemantics;
    public:
        using FilterFunction = std::function<bool(const Semantics*)>;

        static inline const FilterFunction FilterAll = [](const Semantics*) { return true; };

        Semantics(SemanticsObject* sourceObject);

        SemanticsObject* getSourceObject() const;

        virtual const std::string& getName() const = 0;

        virtual bool isSimiliarTo(const Semantics* other) const;
    };

    class DataTypeSemantics : public Semantics
    {
        DataType* m_dataType;
        SymbolTable* m_symbolTable;
    public:
        DataTypeSemantics(SemanticsObject* sourceObject, DataType* dataType, SymbolTable* symbolTable);

        const std::string& getName() const override;

        bool isSimiliarTo(const Semantics* other) const override;

        DataType* getDataType() const;

        SymbolTable* getSymbolTable() const;

        using DataTypeFilterFunction = std::function<bool(const DataTypeSemantics*)>;

        static FilterFunction Filter(const DataType* dataType);

        static FilterFunction Filter(const DataTypeFilterFunction& filter = [](const DataTypeSemantics*){ return true; });
    };
};