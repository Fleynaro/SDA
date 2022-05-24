#pragma once
#include <map>
#include "PcodeBlock.h"

namespace sda::pcode
{
    class Graph
    {
        std::map<InstructionOffset, Instruction> m_instructions;
        std::map<InstructionOffset, Block> m_blocks;
    public:
        void addInstruction(const Instruction& instruction);

        const Instruction* getInstructionAt(InstructionOffset offset) const;

        Block* createBlock(InstructionOffset offset);

        // Clean the block from its instructions and disconnect it from the graph
        void cleanBlock(Block* block);

        Block* getBlockAt(InstructionOffset offset, bool halfInterval = true);

    private:
        void removeInstruction(const Instruction* instruction);
    };
};