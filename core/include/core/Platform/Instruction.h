#pragma once
#include <list>
#include <string>
#include <ostream>

namespace sda
{
    // This instruction is used for printing
    struct Instruction {
        struct Token {
            enum Type
            {
                Mneumonic,
                Register,
                AddressAbs,
                AddressRel,
                Other
            } type;
            std::string text;
        };
        
        std::list<Token> m_tokens;
        int m_length;

        class Printer {
        public:
            virtual void print(const Instruction* instruction) const;

        protected:
			virtual void printToken(const std::string& text, Token::Type token) const = 0;
        };

        class StreamPrinter : public Printer {
            std::ostream& m_output;
        public:
            StreamPrinter(std::ostream& output);

        protected:
            void printToken(const std::string& text, Token::Type token) const override;
        };
    };
};