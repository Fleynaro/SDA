#include "Disasm/PcodeGraphBuilder.h"
#include "Disasm/FunctionCallLookup.h"
#include "Disasm/VtableLookup.h"
#include "Core/Utils.h"

using namespace sda;
using namespace sda::disasm;

PcodeGraphBuilder::PcodeGraphBuilder(pcode::Graph* graph, Image* image, DecoderPcode* decoder)
    : m_graph(graph), m_image(image), m_blockBuilder(graph, image, decoder)
{
    m_callbacks = std::make_unique<Callbacks>();
}

PcodeBlockBuilder* PcodeGraphBuilder::getBlockBuilder() {
    return &m_blockBuilder;
}

void PcodeGraphBuilder::start(const std::list<pcode::InstructionOffset>& startOffsets)
{
    auto oldCallbacks = m_blockBuilder.setCallbacks(nullptr);

    // function call lookup
    std::list<std::pair<pcode::InstructionOffset, pcode::InstructionOffset>> constFunctionOffsets;
    auto funcCallLookupCallbacks = std::make_unique<FunctionCallLookupCallbacks>(&constFunctionOffsets, &m_blockBuilder, std::move(oldCallbacks));
    
    // builder for jumps
    auto stdBuilderCallbacks = std::make_unique<PcodeBlockBuilder::StdCallbacks>(&m_blockBuilder, std::move(funcCallLookupCallbacks));

    // set new callbacks
    m_blockBuilder.setCallbacks(std::move(stdBuilderCallbacks));

    // start the block builder with the callbacks above
    for (auto startOffset : startOffsets) {
        m_blockBuilder.addUnvisitedOffset(startOffset);
    }
    m_blockBuilder.start();

    // create the function graphs
    for (const auto& [_, funcOffset] : constFunctionOffsets) {
        if (auto entryBlock = m_graph->getBlockAt(funcOffset)) {
            if (!entryBlock->getFunctionGraph()) {
                // todo: check references to entry block (JMP -> CALL)
                m_graph->createFunctionGraph(entryBlock);
            }
        }
    }

    // create function references (after the function graphs are created)
    for (const auto& [fromOffset, toOffset] : constFunctionOffsets) {
        if (auto fromBlock = m_graph->getBlockAt(fromOffset)) {
            if (auto fromFuncGraph = fromBlock->getFunctionGraph()) {
                if (auto toEntryBlock = m_graph->getBlockAt(toOffset)) {
                    if (auto toFuncGraph = toEntryBlock->getFunctionGraph()) {
                        fromFuncGraph->addReferencedGraphFrom(fromOffset, toFuncGraph);
                    }
                }
            }
        }
    }

    // update affected blocks (after the function graphs are created)
    for (auto block : stdBuilderCallbacks->getAffectedBlocks()) {
        block->update();
    }
}

std::unique_ptr<PcodeGraphBuilder::Callbacks> PcodeGraphBuilder::setCallbacks(std::unique_ptr<Callbacks> callbacks) {
    auto oldCallbacks = std::move(m_callbacks);
    m_callbacks = std::move(callbacks);
    return oldCallbacks;
}

PcodeGraphBuilder::Callbacks* PcodeGraphBuilder::getCallbacks() const {
    return m_callbacks.get();
}