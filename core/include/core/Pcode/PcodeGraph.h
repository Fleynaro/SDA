#pragma once
#include "PcodeFunctionGraph.h"

namespace sda::pcode
{
    class Graph
    {
        std::map<InstructionOffset, Instruction> m_instructions;
        std::map<InstructionOffset, Block> m_blocks;
        std::map<InstructionOffset, FunctionGraph> m_functionGraphs;
    public:
        void addInstruction(const Instruction& instruction);

        void removeInstruction(const Instruction* instruction);

        const Instruction* getInstructionAt(InstructionOffset offset) const;

        Block* createBlock(InstructionOffset offset);

        // Disconnect the block from the graph and clean/remove it
        void removeBlock(Block* block);
        
        // Split the block at the given offset into 2 blocks and return the new second block
        Block* splitBlock(Block* block, InstructionOffset offset);

        // Join the given blocks
        void joinBlocks(Block* block1, Block* block2);

        Block* getBlockAt(InstructionOffset offset, bool halfInterval = true);

        FunctionGraph* createFunctionGraph(Block* entryBlock);

        void removeFunctionGraph(FunctionGraph* functionGraph);

        const std::map<InstructionOffset, FunctionGraph>& getFunctionGraphs() const;
    };
};