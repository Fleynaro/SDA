#pragma once
#include "PcodeBlock.h"

namespace sda::pcode
{
    class FunctionGraph
    {
        Block* m_entryBlock = nullptr;
        std::list<FunctionGraph*> m_referencedGraphsTo;
        std::map<InstructionOffset, FunctionGraph*> m_referencedGraphsFrom;
    public:
        FunctionGraph() = default;

        FunctionGraph(Block* entryBlock);

        Block* getEntryBlock() const;

        struct BlockInfo {
            Block* block;
            size_t level;
        };

        std::list<BlockInfo> getBlocks() const;

        Graph* getGraph();

        const std::list<FunctionGraph*>& getReferencesTo() const;

        const std::map<InstructionOffset, FunctionGraph*>& getReferencesFrom() const;

        void addReferenceFrom(InstructionOffset fromOffset, FunctionGraph* referencedGraph);

        void addReferenceFrom(InstructionOffset fromOffset, Block* calledBlock);

        void removeAllReferences();

        void moveReferences(
            FunctionGraph* destGraph,
            InstructionOffset startFromOffset = InvalidOffset,
            InstructionOffset endFromOffset = InvalidOffset);
    };
};
