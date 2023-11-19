#pragma once
#include "SDA/Core/Object/ObjectList.h"
#include "SDA/Core/Offset.h"

namespace sda
{
    class Symbol;

    class SymbolTable : public ContextObject
    {
    public:
        static inline const std::string Class = "symbol_table";

        SymbolTable(Context* context, Object::Id* id = nullptr, const std::string& name = "");

        virtual size_t getUsedSize() const = 0;

        virtual void addSymbol(Offset offset, Symbol* symbol) = 0;

        virtual void removeSymbol(Offset offset) = 0;

        struct SymbolInfo {
            SymbolTable* symbolTable = nullptr;
            Offset requestedOffset = 0;
            Symbol* symbol = nullptr;
        };

        virtual std::list<SymbolInfo> getAllSymbols() = 0;

        virtual SymbolInfo getSymbolAt(Offset offset) = 0;

        std::list<SymbolInfo> getAllSymbolsRecursivelyAt(Offset offset, bool includeEmptySymbol = false);

        void serialize(boost::json::object& data) const override;

        void destroy() override;
    };

    class SymbolTableList : public ObjectList<SymbolTable>
    {
    public:
        using ObjectList<SymbolTable>::ObjectList;
    };

    // When symbol is removed from symbol table
    struct SymbolTableSymbolRemovedEvent : Event {
        SymbolTable* symbolTable;
        Offset offset;
        Symbol* symbol;

        SymbolTableSymbolRemovedEvent(SymbolTable* symbolTable, Offset offset, Symbol* symbol)
            : Event(ContextEventTopic)
            , symbolTable(symbolTable)
            , offset(offset)
            , symbol(symbol)
        {}
    };
};