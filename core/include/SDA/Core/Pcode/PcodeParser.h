#pragma once
#include <list>
#include "SDA/Core/Utils/AbstractParser.h"
#include "PcodeInstruction.h"

namespace sda::pcode
{
    class Parser : public utils::AbstractParser
    {
        struct Jump {
            InstructionOffset startOffset;
            InstructionOffset endOffset;
        };
        InstructionOffset m_curOffset = 0;
        std::map<InstructionOffset, Instruction> m_instructions;
        std::map<std::string, Jump> m_labelToJump;
        const RegisterRepository* m_regRepo;
    public:
        Parser(utils::lexer::Lexer* lexer, const RegisterRepository* regRepo);

        static std::list<Instruction> Parse(const std::string& text, const RegisterRepository* regRepo);

        std::list<Instruction> parse(char endSymbol = utils::lexer::EndSymbol);

    private:
        void parseInstruction();

        InstructionId parseInstructionId();

        std::shared_ptr<Varnode> parseVarnode();

        std::shared_ptr<RegisterVarnode> parseRegisterVarnode();

        std::shared_ptr<ConstantVarnode> parseConstantVarnode();

        size_t parseVarnodeSize();

        void parseLabelIfExists();

        void applyLabelJumps();
    };
};