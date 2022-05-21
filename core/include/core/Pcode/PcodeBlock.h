#pragma once
#include <list>
#include "PcodeInstruction.h"

namespace sda::pcode
{
    class Block
    {
        std::list<Instruction*> m_instructions;
        Block* m_nearNextBlock;
        Block* m_farNextBlock;
    public:
        Block(const std::list<Instruction*>& instructions);

        const std::list<Instruction*>& getInstructions() const;

        Block* getNearNextBlock() const;

        Block* getFarNextBlock() const;
    };
};