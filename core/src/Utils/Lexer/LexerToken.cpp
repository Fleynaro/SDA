#include "Core/Utils/Lexer/LexerToken.h"

using namespace utils::lexer;

bool Token::isKeyword(const std::string& keyword) const {
    return false;
}

bool Token::isSymbol(char symbol) const {
    return false;
}

bool Token::isEnd() const {
    return isSymbol('\0');
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