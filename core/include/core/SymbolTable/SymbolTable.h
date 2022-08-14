#pragma once
#include "Core/Object/ObjectList.h"
#include "Core/Offset.h"

namespace sda
{
    class Symbol;

    class SymbolTable : public ContextObject
    {
    public:
        static inline const std::string Collection = "symbol_tables";

        SymbolTable(Context* context, Object::Id* id = nullptr, const std::string& name = "");

        virtual void addSymbol(Offset offset, Symbol* symbol) = 0;

        virtual void removeSymbol(Offset offset) = 0;

        struct SymbolInfo {
            SymbolTable* symbolTable;
            Offset symbolOffset;
            Symbol* symbol;
        };

        virtual SymbolInfo getSymbolAt(Offset offset) = 0;

        std::list<SymbolInfo> getAllSymbolsRecursivelyAt(Offset offset);

        void serialize(boost::json::object& data) const override;

        void destroy() override;
    };

    class SymbolTableList : public ObjectList<SymbolTable>
    {
    public:
        using ObjectList<SymbolTable>::ObjectList;
    };
};