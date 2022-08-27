#pragma once
#include "PcodeInstruction.h"

namespace sda::pcode
{
    class Printer
    {
        const RegisterRepository* m_regRepo;
        size_t m_commentingCounter = 0;
    public:
        Printer(const RegisterRepository* regRepo);

        virtual void printInstruction(const Instruction* instruction) const;

        virtual void printVarnode(const Varnode* varnode, bool printSizeAndOffset = true) const;
        
        void commenting(bool toggle);

    protected:
        enum class Token {
            Mnemonic,
            Register,
            VirtRegister,
            Number,
            Comment,
            Other
        };

        void printToken(const std::string& text, Token token) const;

        virtual void printTokenImpl(const std::string& text, Token token) const = 0;
    };

    class StreamPrinter : public Printer
    {
        std::ostream& m_output;
    public:
        StreamPrinter(std::ostream& output, const RegisterRepository* regRepo);

    protected:
        void printTokenImpl(const std::string& text, Token token) const override;
    };
};