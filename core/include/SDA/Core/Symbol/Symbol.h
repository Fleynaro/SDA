#pragma once
#include "SDA/Core/Object/ObjectList.h"
#include "SDA/Core/Offset.h"

namespace sda
{
    class DataType;
    class SymbolTable;

    class Symbol : public ContextObject
    {
        DataType* m_dataType;
        Offset m_offset = 0;
    protected:
        Symbol(
            Context* context,
            Object::Id* id = nullptr,
            const std::string& name = "",
            DataType* dataType = nullptr);
            
    public:
        static inline const std::string Class = "symbol";

        ~Symbol();

        SymbolTable* getSymbolTable() const;

        Offset getOffset() const;

        void setSymbolTable(SymbolTable* symbolTable, Offset offset);

        void unsetSymbolTable();

        DataType* getDataType() const;

        void setDataType(DataType* dataType);

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;

        void destroy() override;
    };

    class SymbolList : public ObjectList<Symbol>
    {
    public:
        using ObjectList<Symbol>::ObjectList;
    };

    // When symbol table of symbol is unset
    struct SymbolTableUnsetEvent : Event {
        Symbol* symbol;
        SymbolTable* symbolTable;
        Offset offset;

        SymbolTableUnsetEvent(Symbol* symbol, SymbolTable* symbolTable, Offset offset)
            : Event(ContextEventTopic)
            , symbol(symbol)
            , symbolTable(symbolTable)
            , offset(offset)
        {}
    };
};