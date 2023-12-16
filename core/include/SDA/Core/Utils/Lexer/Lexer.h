#pragma once
#include <map>
#include "SDA/Core/Utils/Lexer/LexerIO.h"
#include "SDA/Core/Utils/Lexer/LexerToken.h"

namespace utils::lexer
{
    class Lexer
    {
        IO* m_io;
        char m_letter;
        bool m_skipComment;
    public:
        Lexer(IO* io, bool skipComment = true);
        
        std::unique_ptr<Token> nextToken();

        IO* getIO() const;

    private:
        std::unique_ptr<Token> nextTokenInternal();

        std::unique_ptr<Token> nextIdentToken();

        std::unique_ptr<Token> nextNumberToken();

        std::unique_ptr<Token> nextOtherToken();

        void nextLetter();
    };
};