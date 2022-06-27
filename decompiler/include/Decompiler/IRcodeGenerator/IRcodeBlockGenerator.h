#pragma once
#include "Core/IRcode/IRcodeBlock.h"
#include "Core/Pcode/PcodeInstruction.h"

namespace sda::decompiler
{
    class IRcodeBlockGenerator
    {
        ircode::Block* m_block;
    public:
        IRcodeBlockGenerator(ircode::Block* block);

        void executePcode(pcode::Instruction* instr);
    };
};