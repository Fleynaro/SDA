#include "Core/Utils/AbstractParser.h"

using namespace utils;
using namespace utils::lexer;

AbstractParser::Exception::Exception(const std::string& message)
    : std::exception(message.c_str())
{}

void AbstractParser::init(std::unique_ptr<Token> firstToken) {
    if (firstToken) {
        m_token = std::move(firstToken);
    } else {
        nextToken();
    }
}

std::unique_ptr<Token>& AbstractParser::getToken() {
    return m_token;
}

AbstractParser::AbstractParser(utils::lexer::Lexer* lexer, size_t prefixErrorCode)
    : m_lexer(lexer), m_prefixErrorCode(prefixErrorCode)
{}

utils::lexer::Lexer* AbstractParser::getLexer() const {
    return m_lexer;
}

void AbstractParser::accept(const std::string& keyword) {
    if (!m_token->isKeyword(keyword))
        throw error(200, "Expected keyword '" + keyword + "'");
    nextToken();
}

void AbstractParser::accept(char symbol) {
    if (!m_token->isSymbol(symbol))
        throw error(700, "Expected symbol '" + std::string(1, symbol) + "'");
    nextToken();
}

AbstractParser::Exception AbstractParser::error(ErrorCode code, const std::string& message) {
    m_lexer->getIO()->error(code + m_prefixErrorCode * 1000, message);
    return Exception(message);
}

void AbstractParser::nextToken() {
    m_token = m_lexer->nextToken();
}