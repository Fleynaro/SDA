#pragma once
#include "SDA/Core/DataType/DataType.h"
#include "SDA/Core/SymbolTable/SymbolTable.h"
#include "SDA/Core/Utils/AbstractParser.h"

namespace sda
{
    class DataTypeParser;
    class SymbolTableParser : public utils::AbstractParser
    {
    public:
        struct SymbolTableInfo {
            size_t size = 0;
            std::function<SymbolTable*(SymbolTable*)> create;
        };
        
        struct SymbolInfo {
            std::string name;
            std::string comment;
            size_t size = 0;
            Offset offset = -1;
            std::function<DataType*()> createDataType;
        };

    private:
        DataTypeParser* m_dataTypeParser;
        Context* m_context;
        bool m_isStruct;
        bool m_removeSymbols;

    public:
        SymbolTableParser(
            utils::lexer::Lexer* lexer,
            Context* context,
            DataTypeParser* dataTypeParser = nullptr,
            bool isStruct = false,
            bool removeSymbols = true);

        static SymbolTable* Parse(
            const std::string& text,
            Context* context,
            bool isStruct = false,
            bool withName = true,
            SymbolTable* symbolTable = nullptr,
            bool removeSymbols = true);

        SymbolTableInfo parse(bool withName = true);

    private:
        SymbolInfo parseSymbolDef();
    };
};