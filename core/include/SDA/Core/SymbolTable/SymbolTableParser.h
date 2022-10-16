#pragma once
#include "SDA/Core/SymbolTable/SymbolTable.h"
#include "SDA/Core/Utils/AbstractParser.h"

namespace sda
{
    class SymbolTableParser : public utils::AbstractParser
    {
        Context* m_context;
        bool m_isStruct;
    public:
        SymbolTableParser(utils::lexer::Lexer* lexer, Context* context, bool isStruct = false);

        static SymbolTable* Parse(
            const std::string& text,
            Context* context,
            bool isStruct = false,
            bool withName = true);

        SymbolTable* parse(bool withName = true);

    private:
        Symbol* parseSymbolDef();
    };
};