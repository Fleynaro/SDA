#pragma once
#include "SymbolTable.h"

namespace sda
{   
    class StandartSymbolTable;

    // Optimized symbol table stores symbols in multiple tables.
    class OptimizedSymbolTable : public SymbolTable
    {
        Offset m_minOffset;
        Offset m_maxOffset;
        std::vector<StandartSymbolTable*> m_symbolTables;
    public:
        static inline const std::string Type = "optimized";

        OptimizedSymbolTable(
            Context* context,
            Object::Id* id = nullptr,
            const std::string& name = "",
            Offset minOffset = 0,
            Offset maxOffset = 0,
            size_t fragmentsCount = 0);

        void setFragmentsCount(size_t count);

        size_t getUsedSize() const override;

        void addSymbol(Offset offset, Symbol* symbol) override;

        void removeSymbol(Offset offset) override;

        SymbolInfo getSymbolAt(Offset offset) override;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;

        void destroy() override;

    private:
        StandartSymbolTable* getSymbolTable(Offset offset) const;
    };
};