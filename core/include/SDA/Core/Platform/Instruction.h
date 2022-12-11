#pragma once
#include <list>
#include <string>
#include <ostream>
#include "SDA/Core/Utils/AbstractPrinter.h"

namespace sda
{
    // This instruction is used for printing
    struct Instruction {
        enum Type
        {
            None,
            ConditionalBranch,
            UnconditionalBranch,
        };

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
        
        Type type = None;
        int length = 0;
        size_t jmpOffsetDelta = 0;
        std::list<Token> tokens;

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