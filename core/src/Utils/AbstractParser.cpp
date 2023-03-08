#include "SDA/Core/Utils/AbstractParser.h"

using namespace utils;
using namespace utils::lexer;

AbstractParser::Exception::Exception(ErrorCode code, const std::string& message)
    : std::exception(message.c_str()), code(code)
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

std::string AbstractParser::parseCommentIfExists() {
    std::string comment;
    if (getToken()->isSymbol('[')) {
        nextToken();
        if (auto constToken = dynamic_cast<const ConstToken*>(getToken().get())) {
            if (constToken->valueType == ConstToken::ValueType::String) {
                comment = constToken->value.string;
                nextToken();
            }
        }
        accept(']');
    }
    return comment;
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
    auto resultCode = code + m_prefixErrorCode * 1000;
    m_lexer->getIO()->error(resultCode, message);
    return Exception(resultCode, message);
}

void AbstractParser::nextToken() {
    m_prevToken = std::move(m_token);
    if (m_nextToken) {
        m_token = std::move(m_nextToken);
    } else {
        m_token = m_lexer->nextToken();
    }
}

void AbstractParser::prevToken() {
    m_nextToken = std::move(m_token);
    m_token = std::move(m_prevToken);
}