#pragma once
#include <vector>
#include "PcodeFunctionGraph.h"
#include "PcodeInstructionProvider.h"
#include "SDA/Core/Platform/Platform.h"
#include "SDA/Core/Event/EventPipe.h"

namespace sda::pcode
{
    InstructionOffset GetTargetOffset(const Instruction* instr);

    class Graph
    {
        enum class UpdateBlockState {
            Enabled,
            Disabled,
            DisabledByUpdater
        };

        friend void Block::update();
        friend FunctionGraph; 
        std::map<InstructionOffset, Instruction> m_instructions;
        std::map<InstructionOffset, Block> m_blocks;
        std::map<InstructionOffset, FunctionGraph> m_functionGraphs;
        UpdateBlockState m_updateBlockState = UpdateBlockState::Enabled;
        size_t m_commitLevel = 0;
        std::shared_ptr<EventPipe> m_eventPipe;
        Platform* m_platform;
    public:
        Graph(std::shared_ptr<EventPipe> eventPipe, Platform* platform);

        std::shared_ptr<EventPipe> getEventPipe();

        void explore(InstructionOffset startOffset, InstructionProvider* instrProvider);

        void addInstruction(const Instruction& Instruction, InstructionOffset nextOffset);

        void removeInstruction(const Instruction* instruction);

        const Instruction* getInstructionAt(InstructionOffset offset) const;

        std::vector<const Instruction*> getInstructionsAt(Offset byteOffset) const;

        Block* createBlock(InstructionOffset offset);

        // Disconnect the block from the graph and clean/remove it
        void removeBlock(Block* block);
        
        // Split the block at the given offset into 2 blocks and return the new second block
        Block* splitBlock(Block* block, InstructionOffset offset);

        // Join the given blocks
        void joinBlocks(Block* block1, Block* block2);

        Block* getBlockAt(InstructionOffset offset, bool halfInterval = true);

        Block* getBlockByName(const std::string& name);

        FunctionGraph* getFunctionGraphAt(InstructionOffset offset);

        FunctionGraph* createFunctionGraph(Block* entryBlock);

        void removeFunctionGraph(FunctionGraph* functionGraph);

        void setUpdateBlocksEnabled(bool enabled);
    };
};