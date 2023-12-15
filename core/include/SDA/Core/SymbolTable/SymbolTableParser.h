#pragma once
#include "SDA/Core/SymbolTable/SymbolTable.h"
#include "SDA/Core/Utils/AbstractParser.h"

namespace sda
{
    class DataTypeParser;
    class SymbolTableParser : public utils::AbstractParser
    {
        struct SymbolTableInfo {
            size_t size = 0;
            std::function<SymbolTable*()> create;
        };
        struct SymbolInfo {
            size_t size = 0;
            Offset offset = -1;
            std::function<Symbol*()> create;
        };
        DataTypeParser* m_dataTypeParser;
        Context* m_context;
        bool m_isStruct;
    public:
        SymbolTableParser(
            utils::lexer::Lexer* lexer,
            Context* context,
            DataTypeParser* dataTypeParser = nullptr,
            bool isStruct = false);

        static SymbolTable* Parse(
            const std::string& text,
            Context* context,
            bool isStruct = false,
            bool withName = true,
            SymbolTable* symbolTable = nullptr);

        SymbolTableInfo parse(bool withName = true, SymbolTable* symbolTable = nullptr);

    private:
        SymbolInfo parseSymbolDef();
    };
};