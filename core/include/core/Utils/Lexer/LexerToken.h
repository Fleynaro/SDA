#pragma once
#include <string>
#include <memory>

namespace utils::lexer
{
    struct Token {
        enum Type {
            Ident,
            Keyword,
            Symbol,
            Const
        };

        Type type;

        virtual bool isKeyword(size_t id) const;

        virtual bool isSymbol(char symbol) const;
        
        virtual std::string toString() const = 0;
    };

    struct TokenIdent : Token {
        std::string name;

        std::string toString() const override;
    };

    struct TokenKeyword : Token {
        size_t id;
        std::string name;

        bool isKeyword(size_t id) const override;

        std::string toString() const override;
    };

    struct TokenSymbol : Token {
        char symbol;

        bool isSymbol(char symbol) const override;

        std::string toString() const override;
    };

    struct TokenConst : Token {
        enum ValueType {
            Integer,
            Real,
            String
        };

        ValueType valueType;

        struct {
            size_t integer = 0;
            double real = 0.0;
            std::string string;
        } value;

        std::string toString() const override;
    };
};