#pragma once
#include <map>
#include "Core/Utils/Lexer/LexerIO.h"
#include "Core/Utils/Lexer/LexerToken.h"

namespace utils::lexer
{
    class Lexer
    {
        IO* m_io;
        char m_letter;
        std::map<std::string, size_t> m_keywords;
    public:
        Lexer(IO* io, const std::map<std::string, size_t>& keywords);
        
        std::unique_ptr<Token> nextToken();

    private:
        std::unique_ptr<Token> nextTokenInternal();

        std::unique_ptr<Token> nextIdentOrKeywordToken();

        std::unique_ptr<Token> nextNumberToken();

        std::unique_ptr<Token> nextOtherToken();

        void nextLetter();
    };
};