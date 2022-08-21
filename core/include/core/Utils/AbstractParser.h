#pragma once
#include <map>
#include "Core/Utils/Lexer/Lexer.h"

namespace utils
{
    class AbstractParser
    {
        size_t m_prefixErrorCode;
        utils::lexer::Lexer* m_lexer;
        std::unique_ptr<utils::lexer::Token> m_token;
    public:
        class Exception : public std::exception {
        public:
            Exception(const std::string& message);
        };

        void init(std::unique_ptr<utils::lexer::Token> firstToken = nullptr);

        std::unique_ptr<utils::lexer::Token>& getToken();
    protected:
        AbstractParser(utils::lexer::Lexer* lexer, size_t prefixErrorCode);

        utils::lexer::Lexer* getLexer() const;

        void accept(const std::string& keyword);

        void accept(char symbol);

        Exception error(utils::lexer::ErrorCode code, const std::string& message);

        void nextToken();
    };
};