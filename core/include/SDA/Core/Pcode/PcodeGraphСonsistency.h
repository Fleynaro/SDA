#pragma once
#include <vector>
#include "PcodeGraph.h"

namespace sda::pcode
{
    class GraphConsistency : public Graph::Callbacks
    {
        Graph* m_graph;
    public:
        GraphConsistency(Graph* graph);

    private:
        void onInstructionAdded(const Instruction* instruction, InstructionOffset nextOffset) override;

        void onInstructionRemoved(const Instruction* instruction) override;

        void onBlockCreated(Block* block) override;

        void onBlockRemoved(Block* block) override;

        void onFunctionGraphCreated(FunctionGraph* functionGraph) override;

        void onFunctionGraphRemoved(FunctionGraph* functionGraph) override;

        InstructionOffset getTargetOffset(const pcode::Instruction* instr);
    };
};