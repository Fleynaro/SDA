#pragma once
#include "PcodeBlock.h"

namespace sda::pcode
{
    class FunctionGraph
    {
        friend class Block;
        Block* m_entryBlock = nullptr;
        std::list<FunctionGraph*> m_referencedGraphsTo;
        std::map<InstructionOffset, FunctionGraph*> m_referencedGraphsFrom;
        std::map<size_t, Block*> m_indexToBlock;
    public:
        FunctionGraph() = default;

        FunctionGraph(Block* entryBlock);

        std::string getName() const;

        Block* getEntryBlock() const;

        Offset getEntryOffset() const;

        struct BlockInfo {
            Block* block;
            size_t level;
        };

        std::list<BlockInfo> getBlocks(bool sort = false) const;

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

        std::list<Block*> toBlocks(const utils::BitSet& blockSet) const;
    };
};
