#pragma once
#include "Core/SymbolTable/SymbolTable.h"
#include "Core/Utils/AbstractParser.h"

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
            bool withName = false);

        SymbolTable* parse(bool withName = false);

    private:
        Symbol* parseSymbolDef();
    };
};