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
        public:
            PcodeGraphCallbacks(Program* program) : m_program(program) {}

            void onBlockUpdatedImpl(pcode::Block* pcodeBlock) override;

            void onBlockFunctionGraphChangedImpl(pcode::Block* pcodeBlock, pcode::FunctionGraph* oldFunctionGraph, pcode::FunctionGraph* newFunctionGraph) override;

            // void onBlockCreatedImpl(pcode::Block* pcodeBlock) override;

            // void onBlockRemovedImpl(pcode::Block* pcodeBlock) override;

            void onFunctionGraphCreatedImpl(pcode::FunctionGraph* functionGraph) override;

            void onFunctionGraphRemovedImpl(pcode::FunctionGraph* functionGraph) override;

            void onCommitStartedImpl() override;

            void onCommitEndedImpl() override;
        };
        PcodeGraphCallbacks m_pcodeGraphCallbacks;
    public:
        Program(pcode::Graph* graph);

        std::map<pcode::FunctionGraph*, Function>& getFunctions();

        Function* toFunction(pcode::FunctionGraph* functionGraph);
    };
};