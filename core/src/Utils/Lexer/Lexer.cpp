#include "Core/Utils/Lexer/Lexer.h"

using namespace utils::lexer;

Lexer::Lexer(IO* io, const std::map<std::string, size_t>& keywords)
    : m_io(io), m_keywords(keywords)
{
    nextLetter();
}

std::unique_ptr<Token> Lexer::nextToken() {
    auto token = nextTokenInternal();
    while (!token)
        token = nextTokenInternal();
    if (token->isSymbol('\0'))
        return nullptr;
    return token;
}

std::unique_ptr<Token> Lexer::nextTokenInternal() {
    // skip all spaces, tabs and newlines
    while (std::isspace(m_letter))
        nextLetter();

    // parse an identifier or keyword
    if (std::isalpha(m_letter))
        return nextIdentOrKeywordToken();

    // parse a number
    if (std::isdigit(m_letter))
        return nextNumberToken();

    // parse a string constant or symbol
    return nextOtherToken();
}

std::unique_ptr<Token> Lexer::nextIdentOrKeywordToken() {
    std::string name;
    while (std::isalnum(m_letter)) {
        name += m_letter;
        nextLetter();
    }

    auto it = m_keywords.find(name);
    if (it != m_keywords.end()) {
        TokenKeyword token;
        token.type = Token::Keyword;
        token.id = it->second;
        token.name = name;
        return std::make_unique<TokenKeyword>(token);
    }

    TokenIdent token;
    token.type = Token::Ident;
    token.name = name;
    return std::make_unique<TokenIdent>(token);
}

std::unique_ptr<Token> Lexer::nextNumberToken() {
    std::string strNumber;
    auto base = 10;
    
    // parse a base
    if (m_letter == '0') {
        strNumber = m_letter;
        nextLetter();
        if (m_letter == 'x' || m_letter == 'X') {
            base = 16;
            strNumber = "";
            nextLetter();
        } else if (m_letter == 'b' || m_letter == 'B') {
            base = 2;
            strNumber = "";
            nextLetter();
        }
    }

    // parse an integer
    while (std::isdigit(m_letter)) {
        strNumber += m_letter;
        nextLetter();
    }

    if (m_letter == '.') {
        // parse a real
        nextLetter();
        strNumber += '.';
        while (std::isdigit(m_letter)) {
            strNumber += m_letter;
            nextLetter();
        }
        auto real = std::stod(strNumber);
        TokenConst token;
        token.type = Token::Const;
        token.valueType = TokenConst::Real;
        token.value.real = real;
        return std::make_unique<TokenConst>(token);
    }

    auto integer = std::stoull(strNumber, nullptr, base);
    TokenConst token;
    token.type = Token::Const;
    token.valueType = TokenConst::Integer;
    token.value.integer = integer;
    return std::make_unique<TokenConst>(token);
}

std::unique_ptr<Token> Lexer::nextOtherToken() {
    auto startLetter = m_letter;
    if (m_letter != '\0')
        nextLetter();

    switch (startLetter) {
    // comment
    case '/':
        if (m_letter == '/') {
            // skip a comment
            nextLetter();
            while (m_letter != '\n')
                nextLetter();
            return nullptr;
        } else if (m_letter == '*') {
            // skip a multiline comment
            while (true) {
                nextLetter();
                if (m_letter == '*') {
                    nextLetter();
                    if (m_letter == '/') {
                        nextLetter();
                        return nullptr;
                    }
                }
            }
        }
    
    // string constant
    case '\'':
    case '\"': {
        auto quote = startLetter;
        std::string str;
        while (m_letter != quote) {
            if (m_letter == '\\') {
                // special symbol
                nextLetter();
                switch (m_letter) {
                case 'n':
                    str += '\n';
                    nextLetter();
                    break;
                case 't':
                    str += '\t';
                    nextLetter();
                    break;
                case '\'':
                case '\"':
                    str += quote;
                    nextLetter();
                    break;
                case '\\':
                    str += '\\';
                    nextLetter();
                    break;
                default:
                    str += m_letter;
                    break;
                }
            }
            else {
                str += m_letter;
                nextLetter();
            }
        }
        nextLetter();
        TokenConst token;
        token.type = Token::Const;
        token.valueType = TokenConst::String;
        token.value.string = str;
        return std::make_unique<TokenConst>(token);
    }
    }

    // other symbol (+, /, $, etc.)
    TokenSymbol token;
    token.type = Token::Symbol;
    token.symbol = startLetter;
    return std::make_unique<TokenSymbol>(token);
}

void Lexer::nextLetter() {
    m_letter = m_io->nextLetter();
}