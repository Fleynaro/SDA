#pragma once
#include "PcodeInstruction.h"
#include "Core/Utils/AbstractPrinter.h"

namespace sda::pcode
{
    class Printer : public utils::AbstractPrinter
    {
        const RegisterRepository* m_regRepo;
    public:
        static inline const Token MNEMONIC = KEYWORD;
        static inline const Token REGISTER = IDENTIFIER;
        static inline const Token VIRT_REGISTER = IDENTIFIER;

        Printer(const RegisterRepository* regRepo);

        static std::string Print(const Instruction* instruction, const RegisterRepository* regRepo);

        virtual void printInstruction(const Instruction* instruction) const;

        virtual void printVarnode(const Varnode* varnode, bool printSizeAndOffset = true) const;
    };
};