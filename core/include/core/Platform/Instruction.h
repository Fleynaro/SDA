#pragma once
#include <list>
#include <string>
#include <ostream>
#include "Core/Utils/AbstractPrinter.h"

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

        class Printer : public utils::AbstractPrinter {
        public:
            virtual void print(const Instruction* instruction) const;
        };

        class StreamPrinter : public Printer {
            std::ostream& m_output;
        public:
            StreamPrinter(std::ostream& output);

        protected:
            void printTokenImpl(const std::string& text, Token token) const override;
        };
    };
};