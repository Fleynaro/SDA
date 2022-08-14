#pragma once
#include "Core/SymbolTable/SymbolTable.h"

namespace sda::decompiler
{
    class SemanticsObject;

    class Semantics
    {
        SemanticsObject* m_sourceObject;
        size_t m_uncertaintyDegree;
        std::list<Semantics*> m_successors;
        std::list<Semantics*> m_predecessors;
    public:
        Semantics(SemanticsObject* sourceObject, size_t uncertaintyDegree);

        SemanticsObject* getSourceObject() const;

        size_t getUncertaintyDegree() const;

        void addSuccessors(Semantics* sem);

        const std::list<Semantics*>& getSuccessors() const;

        const std::list<Semantics*>& getPredecessors() const;

        virtual const std::string& getName() const = 0;

        virtual bool isSimiliarTo(const Semantics* other) const;

        using FilterFunction = std::function<bool(const Semantics*)>;

        // Get all semantics
        static FilterFunction FilterAll();

        // Get union of two semantics set
        static FilterFunction FilterOr(const FilterFunction& filter1, const FilterFunction& filter2);

        // Get intersection of two semantics set
        static FilterFunction FilterAnd(const FilterFunction& filter1, const FilterFunction& filter2);
    };

    class DataTypeSemantics : public Semantics
    {
        DataType* m_dataType;
    public:
        DataTypeSemantics(SemanticsObject* sourceObject, DataType* dataType, size_t uncertaintyDegree = 0);

        const std::string& getName() const override;

        bool isSimiliarTo(const Semantics* other) const override;

        DataType* getDataType() const;

        using DataTypeFilterFunction = std::function<bool(const DataTypeSemantics*)>;

        static FilterFunction Filter(const DataType* dataType);

        static FilterFunction Filter(const DataTypeFilterFunction& filter = [](const DataTypeSemantics*){ return true; });
    };

    class SymbolTableSemantics : public Semantics
    {
        SymbolTable* m_symbolTable;
    public:
        SymbolTableSemantics(SemanticsObject* sourceObject, SymbolTable* symbolTable, size_t uncertaintyDegree = 0);
        
        const std::string& getName() const override;

        bool isSimiliarTo(const Semantics* other) const override;

        SymbolTable* getSymbolTable() const;

        using SymbolTableFilterFunction = std::function<bool(const SymbolTableSemantics*)>;

        static FilterFunction Filter(const SymbolTableFilterFunction& filter = [](const SymbolTableSemantics*){ return true; });
    };
};