#pragma once
#include "Core/SymbolTable/SymbolTable.h"

namespace sda::decompiler
{
    class SemanticsObject;

    class Semantics
    {
        friend class SemanticsObject;
    public:
        enum CreatorType {
            System,
            User
        };

        struct SourceInfo {
            CreatorType creatorType;
            Semantics* sourceSemantics = nullptr;
        };

        struct MetaInfo {
            size_t uncertaintyDegree = 0; // todo: float? probability?
        };
    private:
        SemanticsObject* m_holder;
        std::shared_ptr<SourceInfo> m_sourceInfo;
        MetaInfo m_metaInfo;
        std::list<Semantics*> m_successors;
        std::list<Semantics*> m_predecessors;
    public:
        Semantics(
            SemanticsObject* holder,
            const std::shared_ptr<SourceInfo>& sourceInfo,
            const MetaInfo& metaInfo = {});

        virtual ~Semantics();

        SemanticsObject* getHolder() const;

        const std::shared_ptr<SourceInfo>& getSourceInfo() const;

        bool isSource(CreatorType creatorType) const;

        const MetaInfo& getMetaInfo() const;

        void addSuccessor(Semantics* sem);

        const std::list<Semantics*>& getSuccessors() const;

        const std::list<Semantics*>& getPredecessors() const;

        virtual const std::string& getName() const = 0;

        virtual bool isSimiliarTo(const Semantics* other) const;

        virtual std::unique_ptr<Semantics> clone(SemanticsObject* holder, const MetaInfo& metaInfo) const = 0;

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
            enum {
                None,
                Load,
                Extract
            } type = None;
            size_t offset = 0;
            size_t size = 0;
            BitMask sliceMask = 0;
        };
    private:
        DataType* m_dataType;
        SliceInfo m_sliceInfo;
    public:
        DataTypeSemantics(
            SemanticsObject* holder,
            const std::shared_ptr<SourceInfo>& sourceInfo,
            DataType* dataType,
            const SliceInfo& sliceInfo = {},
            const MetaInfo& metaInfo = {});

        const std::string& getName() const override;

        bool isSimiliarTo(const Semantics* other) const override;

        std::unique_ptr<Semantics> clone(SemanticsObject* holder, const MetaInfo& metaInfo) const override;

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
        SymbolTableSemantics(
            SemanticsObject* holder,
            const std::shared_ptr<SourceInfo>& sourceInfo,
            SymbolTable* symbolTable,
            const MetaInfo& metaInfo = {});
        
        const std::string& getName() const override;

        bool isSimiliarTo(const Semantics* other) const override;

        std::unique_ptr<Semantics> clone(SemanticsObject* holder, const MetaInfo& metaInfo) const override;

        SymbolTable* getSymbolTable() const;

        using SymbolTableFilterFunction = std::function<bool(const SymbolTableSemantics*)>;

        static FilterFunction Filter(const SymbolTableFilterFunction& filter = [](const SymbolTableSemantics*){ return true; });
    };

    // class NameSemantics : public Semantics
    // {
    //     std::shared_ptr<std::string> m_name;
    //     size_t m_pointerLevel;
    // public:
    //     // todo: check existence of the same name in symbol table / local var
    // };
};