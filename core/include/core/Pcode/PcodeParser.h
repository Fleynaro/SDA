#pragma once
#include <list>
#include "PcodeInstruction.h"

namespace sda::pcode
{
    class Parser {
        utils::lexer::IO* m_io;
        utils::lexer::Lexer m_lexer;
        const RegisterHelper* m_regHelper;
        std::unique_ptr<utils::lexer::Token> m_token;
    public:
        class Exception : public std::exception {
        public:
            Exception(const std::string& message);
        };

        Parser(utils::lexer::IO* io, const RegisterHelper* regHelper);

        std::list<Instruction> parse();

    private:
        Instruction parseInstruction(InstructionOffset offset);

        InstructionId parseInstructionId();

        std::shared_ptr<Varnode> parseVarnode();

        std::shared_ptr<RegisterVarnode> parseRegisterVarnode();

        std::shared_ptr<ConstantVarnode> parseConstantVarnode();

        size_t parseVarnodeSize();

        void accept(char symbol);

        Exception error(utils::lexer::ErrorCode code, const std::string& message);

        void nextToken();
    };
};