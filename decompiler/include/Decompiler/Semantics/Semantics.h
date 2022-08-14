#pragma once
#include "Core/SymbolTable/SymbolTable.h"

namespace sda::decompiler
{
    class SemanticsObject;

    class Semantics
    {
        friend class SemanticsObject;
    public:
        enum DefType {
            SystemDefined,
            UserDefined
        };
    private:
        DefType m_defType;
        SemanticsObject* m_sourceObject;
        std::set<SemanticsObject*> m_holders;
        size_t m_uncertaintyDegree; // todo: float? probability?
        std::list<Semantics*> m_successors;
        std::list<Semantics*> m_predecessors;
    public:
        Semantics(SemanticsObject* sourceObject, size_t uncertaintyDegree);

        SemanticsObject* getSourceObject() const;

        size_t getUncertaintyDegree() const;

        void addSuccessor(Semantics* sem);

        const std::list<Semantics*>& getSuccessors() const;

        const std::list<Semantics*>& getPredecessors() const;

        virtual const std::string& getName() const = 0;

        virtual bool isSimiliarTo(const Semantics* other) const;

        virtual std::unique_ptr<Semantics> clone(size_t uncertaintyDegree = 0) const = 0;

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
    public:
        struct SliceInfo {
            size_t offset = 0;
            size_t size = 0;
        };
    private:
        DataType* m_dataType;
        SliceInfo m_sliceInfo;
    public:
        DataTypeSemantics(SemanticsObject* sourceObject, DataType* dataType, const SliceInfo& sliceInfo, size_t uncertaintyDegree = 0);

        const std::string& getName() const override;

        bool isSimiliarTo(const Semantics* other) const override;

        std::unique_ptr<Semantics> clone(size_t uncertaintyDegree = 0) const override;

        DataType* getDataType() const;

        const SliceInfo& getSliceInfo() const;

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

        std::unique_ptr<Semantics> clone(size_t uncertaintyDegree = 0) const override;

        SymbolTable* getSymbolTable() const;

        using SymbolTableFilterFunction = std::function<bool(const SymbolTableSemantics*)>;

        static FilterFunction Filter(const SymbolTableFilterFunction& filter = [](const SymbolTableSemantics*){ return true; });
    };
};