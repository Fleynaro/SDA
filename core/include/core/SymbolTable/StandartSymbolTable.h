#pragma once
#include "SymbolTable.h"

namespace sda
{
    // Standart symbol table stores symbols in a single table.
    class StandartSymbolTable : public SymbolTable
    {
        std::map<Offset, Symbol*> m_symbols;
    public:
        static inline const std::string Type = "standart";
        
        StandartSymbolTable(Context* context, Object::Id* id = nullptr, const std::string& name = "");

        size_t getUsedSize() const override;

        void addSymbol(Offset offset, Symbol* symbol) override;

        void removeSymbol(Offset offset) override;

        SymbolInfo getSymbolAt(Offset offset) override;

        const std::map<Offset, Symbol*>& getSymbols() const;

        void setSymbols(const std::map<Offset, Symbol*>& symbols);

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};