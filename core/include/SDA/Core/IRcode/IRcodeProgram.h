#pragma once
#include "IRcodeFunction.h"
#include "SDA/Core/Pcode/PcodeGraph.h"

namespace sda::ircode
{
    class Program
    {
        pcode::Graph* m_graph;
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
        Program(pcode::Graph* graph);

        pcode::Graph* getGraph();

        std::map<pcode::FunctionGraph*, Function>& getFunctions();

        Function* toFunction(pcode::FunctionGraph* functionGraph);

        class Callbacks
        {
            std::shared_ptr<Callbacks> m_prevCallbacks;
            bool m_enabled = true;
        public:
            // Called when an operation is added
            void onOperationAdded(const Operation* op, Block* block);

            // Called when an operation is removed
            void onOperationRemoved(const Operation* op, Block* block);

            void setPrevCallbacks(std::shared_ptr<Callbacks> prevCallbacks);

            void setEnabled(bool enabled);

        protected:
            virtual void onOperationAddedImpl(const Operation* op, Block* block) {};

            virtual void onOperationRemovedImpl(const Operation* op, Block* block) {};
        };

        // Set the callbacks for the graph
        void setCallbacks(std::shared_ptr<Callbacks> callbacks);

        // Get the callbacks for the graph
        std::shared_ptr<Callbacks> getCallbacks() const;

    private:
        std::shared_ptr<Callbacks> m_callbacks;
    };
};