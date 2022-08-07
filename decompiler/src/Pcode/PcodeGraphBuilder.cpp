#include "Decompiler/Pcode/PcodeGraphBuilder.h"
#include "Decompiler/Pcode/FunctionCallLookup.h"
#include "Decompiler/Pcode/VtableLookup.h"
#include "Core/Utils/IOManip.h"

using namespace sda;
using namespace sda::decompiler;

PcodeGraphBuilder::PcodeGraphBuilder(pcode::Graph* graph, Image* image, PcodeDecoder* decoder)
    : m_graph(graph), m_image(image), m_blockBuilder(graph, image, decoder)
{
    m_callbacks = std::make_unique<Callbacks>();
}

PcodeBlockBuilder* PcodeGraphBuilder::getBlockBuilder() {
    return &m_blockBuilder;
}

void PcodeGraphBuilder::start(const std::list<pcode::InstructionOffset>& startOffsets, bool fromEntryPoints)
{
    // function call lookup
    auto funcCallLookupCallbacks = std::make_shared<FunctionCallLookupCallbacks>(
        &m_blockBuilder, m_blockBuilder.getCallbacks());
    
    // builder for jumps
    auto stdBuilderCallbacks = std::make_shared<PcodeBlockBuilder::StdCallbacks>(
        &m_blockBuilder, funcCallLookupCallbacks);

    // set new callbacks
    m_blockBuilder.setCallbacks(stdBuilderCallbacks);

    // start the block builder with the callbacks above
    for (auto startOffset : startOffsets) {
        m_blockBuilder.addUnvisitedOffset(startOffset);
    }
    m_blockBuilder.start();

    // create the function graphs
    if (fromEntryPoints) {
        for (auto startOffset : startOffsets) {
            createFunctionGraph(startOffset);
        }
    }
    for (const auto& [_, funcOffset] : funcCallLookupCallbacks->getConstFunctionOffsets()) {
        createFunctionGraph(funcOffset);
    }

    // create function references (after the function graphs are created)
    for (const auto& [fromOffset, toOffset] : funcCallLookupCallbacks->getConstFunctionOffsets()) {
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

void PcodeGraphBuilder::setCallbacks(std::shared_ptr<Callbacks> callbacks) {
    m_callbacks = callbacks;
}

std::shared_ptr<PcodeGraphBuilder::Callbacks> PcodeGraphBuilder::getCallbacks() const {
    return m_callbacks;
}

void PcodeGraphBuilder::createFunctionGraph(pcode::InstructionOffset offset) {
    if (auto entryBlock = m_graph->getBlockAt(offset)) {
        if (!entryBlock->getFunctionGraph()) {
            // todo: check references to entry block (JMP -> CALL)
            m_graph->createFunctionGraph(entryBlock);
        }
    }
}