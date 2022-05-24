#include "Disasm/PcodeGraphBuilder.h"
#include "Core/Utils.h"

using namespace sda;
using namespace sda::disasm;

PcodeGraphBuilder::PcodeGraphBuilder(pcode::Graph* graph, Image* image, DecoderPcode* decoder)
    : m_graph(graph), m_image(image), m_decoder(decoder)
{}

void PcodeGraphBuilder::start() {
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
            m_image->getReader()->readBytesAtOffset(byteOffset, data);
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
        block->getInstructions().push_back(instr);

        // calculate the next offset
        auto nextOffset = pcode::InstructionOffset(offset + 1);
        if (!m_graph->getInstructionAt(offset))
            nextOffset = pcode::InstructionOffset(offset.byteOffset + m_curOrigInstrLength, 0);
        block->setMaxOffset(nextOffset);

        getCallbacks()->onInstructionPassed(instr, nextOffset);
    }
}

void PcodeGraphBuilder::addUnvisitedOffset(pcode::InstructionOffset offset) {
    m_unvisitedOffsets.push_back(offset);
}

void PcodeGraphBuilder::StdCallbacks::onInstructionPassed(const pcode::Instruction* instr, pcode::InstructionOffset nextOffset) {
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
                // split the already existing block into 2 non-empty blocks
                auto block1 = alreadyExistingBlock;
                auto block2 = m_builder->m_graph->createBlock(targetOffset);

                std::list<const pcode::Instruction*> instrOfBlock1;
                std::list<const pcode::Instruction*> instrOfBlock2;
                for (const auto instr : alreadyExistingBlock->getInstructions()) {
                    if (instr->getOffset() < targetOffset)
                        instrOfBlock1.push_back(instr);
                    else instrOfBlock2.push_back(instr);
                }
                block1->getInstructions() = instrOfBlock1;
                block2->getInstructions() = instrOfBlock2;

                block2->setMaxOffset(alreadyExistingBlock->getMaxOffset());
                block1->setMaxOffset(targetOffset);

                if (block1->getNearNextBlock())
                    block2->setNearNextBlock(block1->getNearNextBlock());
                if (block1->getFarNextBlock())
                    block2->setFarNextBlock(block1->getFarNextBlock());
                block1->setNearNextBlock(block2);
                block1->setFarNextBlock(nullptr);

                block->setFarNextBlock(nextFarBlock = block2);
            }
        }
        else {
            nextFarBlock = m_builder->m_graph->createBlock(targetOffset);
            block->setFarNextBlock(nextFarBlock);
        }

        // calculate the next offset (selecting the next following block if possible)
        if (nextFarBlock)
            m_builder->addUnvisitedOffset(nextFarBlock->getMinOffset());
        if (nextNearBlock)
            m_builder->addUnvisitedOffset(nextNearBlock->getMinOffset());
    }
    else if (instr->getId() != pcode::InstructionId::RETURN && instr->getId() != pcode::InstructionId::INT) {
        if (auto nextBlock = m_builder->m_graph->getBlockAt(nextOffset, false)) {
            if (block != nextBlock) {
                block->setNearNextBlock(nextBlock);
            }
        }

        m_builder->addUnvisitedOffset(nextOffset);
    }

    if (m_nextCallbacks)
        m_nextCallbacks->onInstructionPassed(instr, nextOffset);
}

void PcodeGraphBuilder::StdCallbacks::onWarningEmitted(const std::string& message) {
    if (m_nextCallbacks)
        m_nextCallbacks->onWarningEmitted(message);
}

pcode::InstructionOffset PcodeGraphBuilder::StdCallbacks::getTargetOffset(const pcode::Instruction* instr) {
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

void PcodeGraphBuilder::FunctionCallbacks::onInstructionPassed(const pcode::Instruction* instr, pcode::InstructionOffset nextOffset) {
    if (instr->getId() == pcode::InstructionId::CALL) {
        auto targetOffset = getTargetOffset(instr);
        if (targetOffset == InvalidOffset)
            return;
        m_constFunctionOffsets.push_back(targetOffset);
    }

    if (m_nextCallbacks)
        m_nextCallbacks->onInstructionPassed(instr, nextOffset);
}

void PcodeGraphBuilder::VtableCallbacks::onInstructionPassed(const pcode::Instruction* instr, pcode::InstructionOffset nextOffset) {
    if (instr->getId() == pcode::InstructionId::COPY) {
        if (const auto constVarnode = std::dynamic_pointer_cast<pcode::ConstantVarnode>(instr->getInput0())) {
            Offset vtableRva = constVarnode->getValue();
            if (auto section = m_builder->m_image->getImageSectionAt(vtableRva)) {
                if (section->m_type == ImageSection::DATA_SEGMENT) {
                    std::list<Offset> funcOffsets;
                    
                    // assuming it is a vtable (not global var), get the function offsets from it
                    auto offset = m_builder->m_image->toImageFileOffset(vtableRva);
                    while (offset < m_builder->m_image->getSize()) {
                        std::vector<uint8_t> buffer(sizeof Offset);
                        m_builder->m_image->getReader()->readBytesAtOffset(offset, buffer);

                        const auto funcAddr = *reinterpret_cast<Offset*>(buffer.data());
                        if (funcAddr == 0x0) // all vtables ends with zero address
                            break;

                        const auto funcRva = m_builder->m_image->toOffset(funcAddr);
                        auto section = m_builder->m_image->getImageSectionAt(funcRva);
                        if (section && section->m_type == ImageSection::CODE_SEGMENT)
                            funcOffsets.push_back(funcRva);
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