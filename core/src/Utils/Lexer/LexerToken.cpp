#include "Core/Utils/Lexer/LexerToken.h"

using namespace utils::lexer;

bool Token::isKeyword(size_t id) const {
    return false;
}

bool Token::isSymbol(char symbol) const {
    return false;
}

std::string TokenIdent::toString() const {
    return name;
}

bool TokenKeyword::isKeyword(size_t id) const {
    return id == this->id;
}

std::string TokenKeyword::toString() const {
    return name;
}

bool TokenSymbol::isSymbol(char symbol) const {
    return symbol == this->symbol;
}

std::string TokenSymbol::toString() const {
    return std::string(1, symbol);
}

std::string TokenConst::toString() const {
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