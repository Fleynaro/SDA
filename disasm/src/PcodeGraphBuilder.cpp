#include "Disasm/PcodeGraphBuilder.h"
#include "Core/Utils.h"

using namespace sda;
using namespace sda::disasm;

PcodeGraphBuilder::PcodeGraphBuilder(pcode::Graph* graph, Image* image, DecoderPcode* decoder)
    : m_graph(graph), m_image(image), m_decoder(decoder)
{
    m_callbacks = std::make_unique<Callbacks>();
}

void PcodeGraphBuilder::start(
    const std::list<pcode::InstructionOffset>& startOffsets,
    std::unique_ptr<PcodeBlockBuilder::Callbacks> nextCallbacks)
{
    PcodeBlockBuilder blockBuilder(m_graph, m_image, m_decoder);
    
    // builder for functions and vtables
    auto blockBuilderCallbacks = std::make_unique<PcodeGraphBuilder::BlockBuilderCallbacks>();
    blockBuilderCallbacks->m_builder = &blockBuilder;
    blockBuilderCallbacks->m_graphBuilder = this;
    blockBuilderCallbacks->m_image = m_image;
    blockBuilderCallbacks->m_nextCallbacks = std::move(nextCallbacks);
    
    // builder for jumps
    auto stdBuilderCallbacks = std::make_unique<PcodeBlockBuilder::StdCallbacks>();
    stdBuilderCallbacks->m_builder = &blockBuilder;
    stdBuilderCallbacks->m_nextCallbacks = std::move(blockBuilderCallbacks);
    blockBuilder.setCallbacks(std::move(stdBuilderCallbacks));

    // start the block builder with the callbacks above
    for (auto startOffset : startOffsets) {
        blockBuilder.addUnvisitedOffset(startOffset);
    }
    blockBuilder.start();

    // create the function graphs
    for (const auto& [_, funcOffset] : blockBuilderCallbacks->m_constFunctionOffsets) {
        if (auto entryBlock = m_graph->getBlockAt(funcOffset)) {
            if (!entryBlock->getFunctionGraph()) {
                // todo: check references to entry block (JMP -> CALL)
                m_graph->createFunctionGraph(entryBlock);
            }
        }
    }

    // create function references
    for (const auto& [fromOffset, toOffset] : blockBuilderCallbacks->m_constFunctionOffsets) {
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

    // update affected blocks
    for (auto block : stdBuilderCallbacks->m_affectedBlocks) {
        block->update();
    }
}

void PcodeGraphBuilder::BlockBuilderCallbacks::onInstructionPassed(const pcode::Instruction* instr, pcode::InstructionOffset nextOffset) {
    if (instr->getId() == pcode::InstructionId::CALL) {
        auto targetOffset = getTargetOffset(instr);
        if (targetOffset == InvalidOffset)
            return;
        m_constFunctionOffsets.push_back({ instr->getOffset(), targetOffset });
        m_builder->addUnvisitedOffset(targetOffset);
    }
    else if (instr->getId() == pcode::InstructionId::COPY) {
        if (const auto constVarnode = std::dynamic_pointer_cast<pcode::ConstantVarnode>(instr->getInput0())) {
            Offset vtableRva = constVarnode->getValue();
            if (auto section = m_image->getImageSectionAt(vtableRva)) {
                if (section->m_type == ImageSection::DATA_SEGMENT) {
                    std::list<Offset> funcOffsets;
                    
                    // assuming it is a vtable (not global var), get the function offsets from it
                    auto offset = m_image->toImageFileOffset(vtableRva);
                    while (offset < m_image->getSize()) {
                        std::vector<uint8_t> buffer(sizeof Offset);
                        m_image->getReader()->readBytesAtOffset(offset, buffer);

                        const auto funcAddr = *reinterpret_cast<Offset*>(buffer.data());
                        if (funcAddr == 0x0) // all vtables ends with zero address
                            break;

                        const auto funcRva = m_image->toOffset(funcAddr);
                        auto section = m_image->getImageSectionAt(funcRva);
                        if (section && section->m_type == ImageSection::CODE_SEGMENT) {
                            funcOffsets.push_back(funcRva);
                            m_builder->addUnvisitedOffset(pcode::InstructionOffset(funcRva, 0));
                        }
                        offset += sizeof(Offset);
                    }

                    if (funcOffsets.empty())
                        m_vtables.push_back(VTable({vtableRva, funcOffsets}));
                }
            }
        }
    }

    if (m_nextCallbacks)
        m_nextCallbacks->onInstructionPassed(instr, nextOffset);
}

std::unique_ptr<PcodeGraphBuilder::Callbacks> PcodeGraphBuilder::setCallbacks(std::unique_ptr<Callbacks> callbacks) {
    auto oldCallbacks = std::move(m_callbacks);
    m_callbacks = std::move(callbacks);
    return oldCallbacks;
}

PcodeGraphBuilder::Callbacks* PcodeGraphBuilder::getCallbacks() const {
    return m_callbacks.get();
}