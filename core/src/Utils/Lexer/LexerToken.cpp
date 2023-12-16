#include "SDA/Core/Utils/Lexer/LexerToken.h"

using namespace utils::lexer;

bool Token::isIdent(std::string& name) const {
    return false;
}

bool Token::isKeyword(const std::string& keyword) const {
    return false;
}

bool Token::isOneOfKeyword(const std::initializer_list<std::string>& keywords) const {
    for (const auto& keyword : keywords) {
        if (isKeyword(keyword)) {
            return true;
        }
    }
    return false;
}

bool Token::isSymbol(char symbol) const {
    return false;
}

bool Token::isComment(std::string& comment) const {
    return false;
}

bool IdentToken::isIdent(std::string& name) const {
    name = this->name;
    return !name.empty();
}

bool IdentToken::isKeyword(const std::string& keyword) const {
    return name == keyword;
}

std::string IdentToken::toString() const {
    return name;
}

bool SymbolToken::isSymbol(char symbol) const {
    return symbol == this->symbol;
}

std::string SymbolToken::toString() const {
    return std::string(1, symbol);
}

std::string ConstToken::toString() const {
    switch (valueType) {
    case ValueType::Integer:
        return std::to_string(value.integer);
    case ValueType::Real:
        return std::to_string(value.real);
    case ValueType::String:
        return value.string;
    }
    return "";
}

bool CommentToken::isComment(std::string& comment) const {
    comment = this->comment;
    return !comment.empty();
}

std::string CommentToken::toString() const {
    return comment;
}