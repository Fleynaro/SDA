#pragma once
#include <list>
#include "Core/Utils/AbstractParser.h"
#include "PcodeInstruction.h"

namespace sda::pcode
{
    class Parser : public utils::AbstractParser
    {
        const RegisterHelper* m_regHelper;
    public:
        Parser(utils::lexer::Lexer* lexer, const RegisterHelper* regHelper);

        static std::list<Instruction> Parse(const std::string& text, const RegisterHelper* regHelper);

        std::list<Instruction> parse();

    private:
        Instruction parseInstruction(InstructionOffset offset);

        InstructionId parseInstructionId();

        std::shared_ptr<Varnode> parseVarnode();

        std::shared_ptr<RegisterVarnode> parseRegisterVarnode();

        std::shared_ptr<ConstantVarnode> parseConstantVarnode();

        size_t parseVarnodeSize();
    };
};