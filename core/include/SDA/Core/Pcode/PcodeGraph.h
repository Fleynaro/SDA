#pragma once
#include <vector>
#include "PcodeFunctionGraph.h"
#include "PcodeInstructionProvider.h"

namespace sda::pcode
{
    class Graph
    {
        std::shared_ptr<InstructionProvider> m_instructionProvider;
        std::map<InstructionOffset, Instruction> m_instructions;
        std::map<InstructionOffset, Block> m_blocks;
        std::map<InstructionOffset, FunctionGraph> m_functionGraphs;
    public:
        Graph(std::shared_ptr<InstructionProvider> instructionProvider);

        std::shared_ptr<InstructionProvider> getInstructionProvider() const;

        void explore(InstructionOffset startOffset);

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

        FunctionGraph* getFunctionGraphAt(InstructionOffset offset);

        FunctionGraph* createFunctionGraph(Block* entryBlock);

        void removeFunctionGraph(FunctionGraph* functionGraph);

         // Callbacks for the graph (TODO: remove?)
        class Callbacks
        {
        public:
            // Called when an instruction is added to the graph
            virtual void onInstructionAdded(const Instruction* instruction, InstructionOffset nextOffset) {}

            // Called when an instruction is removed from the graph
            virtual void onInstructionRemoved(const Instruction* instruction) {}

            // Called when a block is created
            virtual void onBlockCreated(Block* block) {}

            // Called when a block is removed
            virtual void onBlockRemoved(Block* block) {}

            // Called when a function graph is created
            virtual void onFunctionGraphCreated(FunctionGraph* functionGraph) {}

            // Called when a function graph is removed
            virtual void onFunctionGraphRemoved(FunctionGraph* functionGraph) {}

            // Called when an unvisited offset is found
            virtual void onUnvisitedOffsetFound(InstructionOffset offset) {}

            // Called when a warning is emitted
            virtual void onWarningEmitted(const std::string& warning) {}
        };

        // Set the callbacks for the graph
        void setCallbacks(std::shared_ptr<Callbacks> callbacks);

        // Get the callbacks for the graph
        std::shared_ptr<Callbacks> getCallbacks() const;

    private:
        std::shared_ptr<Callbacks> m_callbacks;
    };
};