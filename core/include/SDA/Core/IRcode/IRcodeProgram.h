#pragma once
#include "IRcodeFunction.h"
#include "SDA/Core/Pcode/PcodeGraph.h"
#include "SDA/Core/SymbolTable/SymbolTable.h"

namespace sda::ircode
{
    class Program
    {
        pcode::Graph* m_graph;
        SymbolTable* m_globalSymbolTable;
        std::map<pcode::FunctionGraph*, Function> m_functions;

        class PcodeGraphCallbacks : public pcode::Graph::Callbacks
        {
            Program* m_program;
            bool m_commitStarted = false;
            std::set<pcode::Block*> m_pcodeBlocksToUpdate;

            void onBlockUpdateRequestedImpl(pcode::Block* pcodeBlock) override;

            void onBlockFunctionGraphChangedImpl(pcode::Block* pcodeBlock, pcode::FunctionGraph* oldFunctionGraph, pcode::FunctionGraph* newFunctionGraph) override;

            // void onBlockCreatedImpl(pcode::Block* pcodeBlock) override;

            // void onBlockRemovedImpl(pcode::Block* pcodeBlock) override;

            void onFunctionGraphCreatedImpl(pcode::FunctionGraph* functionGraph) override;

            void onFunctionGraphRemovedImpl(pcode::FunctionGraph* functionGraph) override;

            void onCommitStartedImpl() override;

            void onCommitEndedImpl() override;

            void updateBlocks();

        public:
            PcodeGraphCallbacks(Program* program) : m_program(program) {}
        };
        std::shared_ptr<PcodeGraphCallbacks> m_pcodeGraphCallbacks;
    public:
        Program(pcode::Graph* graph, SymbolTable* globalSymbolTable);

        pcode::Graph* getGraph();

        SymbolTable* getGlobalSymbolTable();

        std::map<pcode::FunctionGraph*, Function>& getFunctions();

        Function* toFunction(pcode::FunctionGraph* functionGraph);

        Function* getFunctionAt(pcode::InstructionOffset offset);

        std::list<Function*> getFunctionsByCallInstruction(const pcode::Instruction* instr);

        class Callbacks
        {
            std::shared_ptr<Callbacks> m_prevCallbacks;
            bool m_enabled = true;
        public:
            // Called when a function is decompiled
            void onFunctionDecompiled(Function* function, std::list<Block*> blocks);

            // Called when a block is created
            void onBlockCreated(Block* block);

            // Called when a block is removed
            void onBlockRemoved(Block* block);

            // Called when an operation is added
            void onOperationAdded(const Operation* op);

            // Called when an operation is removed
            void onOperationRemoved(const Operation* op);

            void setPrevCallbacks(std::shared_ptr<Callbacks> prevCallbacks);

            void setEnabled(bool enabled);

        protected:
            virtual void onFunctionDecompiledImpl(Function* function, std::list<Block*> blocks) {};

            virtual void onBlockCreatedImpl(Block* block) {};

            virtual void onBlockRemovedImpl(Block* block) {};

            virtual void onOperationAddedImpl(const Operation* op) {};

            virtual void onOperationRemovedImpl(const Operation* op) {};
        };

        // Set the callbacks for the graph
        void setCallbacks(std::shared_ptr<Callbacks> callbacks);

        // Get the callbacks for the graph
        std::shared_ptr<Callbacks> getCallbacks() const;

    private:
        std::shared_ptr<Callbacks> m_callbacks;
    };
};