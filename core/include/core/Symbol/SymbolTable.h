#pragma once
#include "Core/Object/ObjectList.h"
#include "Core/Offset.h"

namespace sda
{
    class Symbol;

    class SymbolTable : public ContextObject
    {
        std::map<Offset, Symbol*> m_symbols;
    public:
        static inline const std::string Collection = "symbol_tables";

        SymbolTable(Context* context, Object::Id* id = nullptr, const std::string& name = "");

        void setSymbols(const std::map<Offset, Symbol*>& symbols);

        const std::map<Offset, Symbol*>& getSymbols() const;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;

        void destroy() override;
    };

    class SymbolTableList : public ObjectList<SymbolTable>
    {
    public:
        using ObjectList<SymbolTable>::ObjectList;
    };
};