#include "Core/Pcode/PcodeFunctionGraph.h"

using namespace sda::pcode;

FunctionGraph::FunctionGraph(Block* entryBlock)
    : m_entryBlock(entryBlock)
{}

Block* FunctionGraph::getEntryBlock() const {
    return m_entryBlock;
}

const std::list<FunctionGraph*>& FunctionGraph::getReferencedGraphsTo() const {
    return m_referencedGraphsTo;
}

const std::map<InstructionOffset, FunctionGraph*>& FunctionGraph::getReferencedGraphsFrom() const {
    return m_referencedGraphsFrom;
}

void FunctionGraph::addReferencedGraphFrom(InstructionOffset fromOffset, FunctionGraph* referencedGraph) {
    m_referencedGraphsFrom[fromOffset] = referencedGraph;
    referencedGraph->m_referencedGraphsTo.push_back(this);
}

void FunctionGraph::removeReferencedGraphFrom(InstructionOffset fromOffset) {
    auto it = m_referencedGraphsFrom.find(fromOffset);
    if (it != m_referencedGraphsFrom.end()) {
        it->second->m_referencedGraphsTo.remove(this);
        m_referencedGraphsFrom.erase(it);
    }
}
