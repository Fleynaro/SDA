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

        virtual bool isKeyword(const std::string& keyword) const;

        virtual bool isSymbol(char symbol) const;

        bool isEnd() const;
        
        virtual std::string toString() const = 0;
    };

    // Identifier or keyword
    struct IdentToken : Token {
        std::string name;

        bool isKeyword(const std::string& keyword) const override;

        std::string toString() const override;
    };

    // Symbol (e.g. '+', '-', '*', '/', '=', ';', etc.)
    struct SymbolToken : Token {
        char symbol;

        bool isSymbol(char symbol) const override;

        std::string toString() const override;
    };

    // Constant (integer, real, string)
    struct ConstToken : Token {
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