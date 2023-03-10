#pragma once
#include <vector>
#include "PcodeFunctionGraph.h"
#include "PcodeInstructionProvider.h"

namespace sda::pcode
{
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
    public:
        struct CommitScope {
            Graph* m_graph;
            CommitScope(Graph* graph) : m_graph(graph) { m_graph->startCommit(); }
            ~CommitScope() { m_graph->endCommit(); }
        };

        Graph();

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

        void startCommit();

        void endCommit();

         // Callbacks for the graph
        class Callbacks
        {
            std::shared_ptr<Callbacks> m_prevCallbacks;
            bool m_enabled = true;
        public:
            // Called when an instruction is added to the graph
            void onInstructionAdded(const Instruction* instruction, InstructionOffset nextOffset);

            // Called when an instruction is removed from the graph
            void onInstructionRemoved(const Instruction* instruction);

            // Called when a block is created
            void onBlockCreated(Block* block);

            // Called when a block is updated
            void onBlockUpdated(Block* block);

            // Called when a block is requested to be updated
            void onBlockUpdateRequested(Block* block);

            // Called when a block's function graph is changed
            void onBlockFunctionGraphChanged(Block* block, FunctionGraph* oldFunctionGraph, FunctionGraph* newFunctionGraph);

            // Called when a block is removed
            void onBlockRemoved(Block* block);

            // Called when a function graph is created
            void onFunctionGraphCreated(FunctionGraph* functionGraph);

            // Called when a function graph is removed
            void onFunctionGraphRemoved(FunctionGraph* functionGraph);

            // Called when an unvisited offset is found
            void onUnvisitedOffsetFound(InstructionOffset offset);

            // Called when the commit starts
            void onCommitStarted();

            // Called when the commit ends
            void onCommitEnded();

            // Called when a warning is emitted
            void onWarningEmitted(const std::string& warning);

            void setPrevCallbacks(std::shared_ptr<Callbacks> prevCallbacks);

            void setEnabled(bool enabled);

        protected:
            virtual void onInstructionAddedImpl(const Instruction* instruction, InstructionOffset nextOffset) {};

            virtual void onInstructionRemovedImpl(const Instruction* instruction) {};

            virtual void onBlockCreatedImpl(Block* block) {};

            virtual void onBlockUpdatedImpl(Block* block) {};

            virtual void onBlockUpdateRequestedImpl(Block* block) {};

            virtual void onBlockFunctionGraphChangedImpl(Block* block, FunctionGraph* oldFunctionGraph, FunctionGraph* newFunctionGraph) {};

            virtual void onBlockRemovedImpl(Block* block) {};

            virtual void onFunctionGraphCreatedImpl(FunctionGraph* functionGraph) {};

            virtual void onFunctionGraphRemovedImpl(FunctionGraph* functionGraph) {};

            virtual void onUnvisitedOffsetFoundImpl(InstructionOffset offset) {};

            virtual void onCommitStartedImpl() {};

            virtual void onCommitEndedImpl() {};

            virtual void onWarningEmittedImpl(const std::string& warning) {};
        };

        // Set the callbacks for the graph
        void setCallbacks(std::shared_ptr<Callbacks> callbacks);

        // Get the callbacks for the graph
        std::shared_ptr<Callbacks> getCallbacks() const;

    private:
        std::shared_ptr<Callbacks> m_callbacks;
    };
};