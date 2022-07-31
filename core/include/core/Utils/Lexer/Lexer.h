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
    public:
        Lexer(IO* io);
        
        std::unique_ptr<Token> nextToken();

    private:
        std::unique_ptr<Token> nextTokenInternal();

        std::unique_ptr<Token> nextIdentToken();

        std::unique_ptr<Token> nextNumberToken();

        std::unique_ptr<Token> nextOtherToken();

        void nextLetter();
    };
};