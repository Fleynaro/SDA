#pragma once
#include <map>
#include "PcodeInstruction.h"

namespace sda::pcode
{
    class Block
    {
        std::map<InstructionOffset, Instruction> m_instructions;
    public:
        Block(const std::map<InstructionOffset, Instruction>& instructions);

        const std::map<InstructionOffset, Instruction>& getInstructions() const;
    };
};