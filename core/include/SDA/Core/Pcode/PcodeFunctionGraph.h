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

        const std::list<FunctionGraph*>& getReferencedGraphsTo() const;

        const std::map<InstructionOffset, FunctionGraph*>& getReferencedGraphsFrom() const;

        void addReferencedGraphFrom(InstructionOffset fromOffset, FunctionGraph* referencedGraph);

        void removeReferencedGraphFrom(InstructionOffset fromOffset);
    };
};