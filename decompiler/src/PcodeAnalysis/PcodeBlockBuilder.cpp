#include "Decompiler/PcodeAnalysis/PcodeGraphBuilder.h"
#include "Core/Utils/IOManip.h"

using namespace sda;
using namespace sda::decompiler;

PcodeBlockBuilder::PcodeBlockBuilder(pcode::Graph* graph, Image* image, disasm::DecoderPcode* decoder)
    : m_graph(graph), m_image(image), m_decoder(decoder)
{
    m_callbacks = std::make_unique<Callbacks>();
}

void PcodeBlockBuilder::start() {
    while (!m_unvisitedOffsets.empty()) {
        // get the next unvisited offset
        const auto offset = m_unvisitedOffsets.back();
        m_unvisitedOffsets.pop_back();

        // if the offset is already visited, skip it, else mark it as visited
        if (m_visitedOffsets.find(offset) != m_visitedOffsets.end())
            continue;
        m_visitedOffsets.insert(offset);

        // decode bytes of the image at this offset to get P-code instructions
        if (offset.index == 0) {
            // if there are instructions at this offset, don't visit it again (offset might not be in m_visitedOffsets!)
            if (m_graph->getInstructionAt(offset))
                continue;

            const auto byteOffset = offset.byteOffset;
            if (byteOffset >= m_image->getSize())
                continue;

            std::vector<uint8_t> data(100);
            m_image->getRW()->readBytesAtOffset(byteOffset, data);
            m_decoder->decode(byteOffset, data);
            for (const auto& instr : m_decoder->getDecodedInstructions())
                m_graph->addInstruction(instr);
            m_curOrigInstrLength = m_decoder->getInstructionLength();
        }

        // get the P-code instruction at this offset
        const auto instr = m_graph->getInstructionAt(offset);
        if (!instr)
            throw std::runtime_error("Instruction not found");

        // get or create a block at this offset
        auto block = m_graph->getBlockAt(offset, false);
        if (!block) {
            block = m_graph->createBlock(offset);
        }

        // add the instruction to the block
        block->getInstructions()[offset] = instr;

        // calculate the next offset
        auto nextOffset = pcode::InstructionOffset(offset + 1);
        if (!m_graph->getInstructionAt(nextOffset))
            nextOffset = pcode::InstructionOffset(offset.byteOffset + m_curOrigInstrLength, 0);
        if (block->getMaxOffset() < nextOffset)
            block->setMaxOffset(nextOffset);

        // handle the instruction
        getCallbacks()->onInstructionPassed(instr, nextOffset);
    }
}

void PcodeBlockBuilder::addUnvisitedOffset(pcode::InstructionOffset offset) {
    m_unvisitedOffsets.push_back(offset);
}

PcodeBlockBuilder::StdCallbacks::StdCallbacks(PcodeBlockBuilder* builder, std::shared_ptr<Callbacks> nextCallbacks)
    : m_builder(builder), m_nextCallbacks(std::move(nextCallbacks))
{}

const std::set<pcode::Block*>& PcodeBlockBuilder::StdCallbacks::getAffectedBlocks() const {
    return m_affectedBlocks;
}

void PcodeBlockBuilder::StdCallbacks::onInstructionPassed(const pcode::Instruction* instr, pcode::InstructionOffset nextOffset) {
    // get the current block again
    auto block = m_builder->m_graph->getBlockAt(instr->getOffset());
    
    if (instr->isBranching()) {
        auto targetOffset = getTargetOffset(instr);
        if (targetOffset == InvalidOffset)
            return;

        // near block
        pcode::Block* nextNearBlock = nullptr;
        if (instr->getId() == pcode::InstructionId::CBRANCH) {
            if (!m_builder->m_graph->getBlockAt(nextOffset)) {
                nextNearBlock = m_builder->m_graph->createBlock(nextOffset);
                block->setNearNextBlock(nextNearBlock);
            }
        }

        // far block
        pcode::Block* nextFarBlock = nullptr;
        if (auto alreadyExistingBlock = m_builder->m_graph->getBlockAt(targetOffset)) {
            if (targetOffset == alreadyExistingBlock->getMinOffset()) {
                block->setFarNextBlock(nextFarBlock = alreadyExistingBlock);
            }
            else {
                nextFarBlock = m_builder->m_graph->splitBlock(alreadyExistingBlock, targetOffset);
                block->setFarNextBlock(nextFarBlock);
            }
        }
        else {
            nextFarBlock = m_builder->m_graph->createBlock(targetOffset);
            block->setFarNextBlock(nextFarBlock);
        }

        // calculate the next offset (selecting the next following (near) block if possible)
        if (nextFarBlock)
            m_builder->addUnvisitedOffset(nextFarBlock->getMinOffset());
        if (nextNearBlock)
            m_builder->addUnvisitedOffset(nextNearBlock->getMinOffset());

        m_affectedBlocks.insert(block);
    }
    else if (instr->getId() != pcode::InstructionId::RETURN && instr->getId() != pcode::InstructionId::INT) {
        if (auto nextBlock = m_builder->m_graph->getBlockAt(nextOffset)) {
            /*
                Case 1:
                    0x... {instructions above}
                    0x100 ADD       <--- end of the current block
                    0x101 INT/RET   <--- here stop
                    0x102 COPY      <--- begin of the next block
                    0x... {instructions below}

                Case 2 (need to join blocks):
                    0x... {instructions above}
                    0x100 ADD       <--- end of the current block
                    0x101 COPY      <--- begin of the next block
                    0x... {instructions below}
            */
            if (block != nextBlock) {
                if (nextBlock->getReferencedBlocks().size() == 0) {
                    // if no other blocks reference this block, join it
                    m_builder->m_graph->joinBlocks(block, nextBlock);
                } else {
                    // otherwise, make a link
                    block->setNearNextBlock(nextBlock);
                }
                m_affectedBlocks.insert(block);
            }
        }

        m_builder->addUnvisitedOffset(nextOffset);
    }

    if (m_nextCallbacks)
        m_nextCallbacks->onInstructionPassed(instr, nextOffset);
}

void PcodeBlockBuilder::StdCallbacks::onWarningEmitted(const std::string& message) {
    if (m_nextCallbacks)
        m_nextCallbacks->onWarningEmitted(message);
}

pcode::InstructionOffset PcodeBlockBuilder::StdCallbacks::getTargetOffset(const pcode::Instruction* instr) {
    pcode::InstructionOffset targetOffset = InvalidOffset;
    if (const auto constVarnode = std::dynamic_pointer_cast<pcode::ConstantVarnode>(instr->getInput0())) {
        // if this input contains a hardcoded constant
        targetOffset = constVarnode->getValue();
    }

    // check if the target offset is valid
    auto section = m_builder->m_image->getImageSectionAt(targetOffset.byteOffset);
    if (targetOffset == InvalidOffset || (section && section->m_type != ImageSection::CODE_SEGMENT)) {
        std::stringstream ss;
        ss << "Invalid target 0x" << utils::to_hex() << targetOffset << " at offset 0x" << utils::to_hex() << instr->getOffset();
        onWarningEmitted(ss.str());
    }

    return targetOffset;
}

void PcodeBlockBuilder::setCallbacks(std::shared_ptr<Callbacks> callbacks) {
    m_callbacks = callbacks;
}

std::shared_ptr<PcodeBlockBuilder::Callbacks> PcodeBlockBuilder::getCallbacks() const {
    return m_callbacks;
}