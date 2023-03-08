#pragma once
#include <map>
#include "SDA/Core/Utils/Lexer/Lexer.h"

namespace utils
{
    class AbstractParser
    {
        size_t m_prefixErrorCode;
        utils::lexer::Lexer* m_lexer;
        std::unique_ptr<utils::lexer::Token> m_token; // current token
        std::unique_ptr<utils::lexer::Token> m_prevToken;
        std::unique_ptr<utils::lexer::Token> m_nextToken;
    public:
        class Exception : public std::exception {
            utils::lexer::ErrorCode code;
        public:
            Exception(utils::lexer::ErrorCode code, const std::string& message);

            utils::lexer::ErrorCode getCode() const { return code; }
        };

        void init(std::unique_ptr<utils::lexer::Token> firstToken = nullptr);

        std::unique_ptr<utils::lexer::Token>& getToken();
    protected:
        AbstractParser(utils::lexer::Lexer* lexer, size_t prefixErrorCode);

        utils::lexer::Lexer* getLexer() const;

        std::string parseCommentIfExists();

        void accept(const std::string& keyword);

        void accept(char symbol);

        Exception error(utils::lexer::ErrorCode code, const std::string& message);

        void nextToken();

        void prevToken();
    };
};