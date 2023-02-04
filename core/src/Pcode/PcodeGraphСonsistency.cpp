#include "SDA/Core/Pcode/PcodeGraphСonsistency.h"
#include "SDA/Core/Utils/IOManip.h"

using namespace sda::pcode;

GraphConsistency::GraphConsistency(Graph* graph)
    : m_graph(graph)
{}

void GraphConsistency::onInstructionAdded(const Instruction* instruction, InstructionOffset nextOffset)
{
    auto offset = instruction->getOffset();

    // get or create a block at this offset
    auto block = m_graph->getBlockAt(offset, false);
    if (!block) {
        block = m_graph->createBlock(offset);
    }
    block->getInstructions()[offset] = instruction;
    if (block->getMaxOffset() < nextOffset)
        block->setMaxOffset(nextOffset);

    if (instruction->isBranching()) {
        auto targetOffset = getTargetOffset(instruction);
        if (targetOffset == InvalidOffset)
            return;

        // near block
        pcode::Block* nextNearBlock = nullptr;
        if (instruction->getId() == pcode::InstructionId::CBRANCH) {
            if (!m_graph->getBlockAt(nextOffset)) {
                nextNearBlock = m_graph->createBlock(nextOffset);
                block->setNearNextBlock(nextNearBlock);
            }
        }

        // far block
        pcode::Block* nextFarBlock = nullptr;
        if (auto alreadyExistingBlock = m_graph->getBlockAt(targetOffset)) {
            if (targetOffset == alreadyExistingBlock->getMinOffset()) {
                block->setFarNextBlock(nextFarBlock = alreadyExistingBlock);
            }
            else {
                nextFarBlock = m_graph->splitBlock(alreadyExistingBlock, targetOffset);
                block->setFarNextBlock(nextFarBlock);
            }
        }
        else {
            nextFarBlock = m_graph->createBlock(targetOffset);
            block->setFarNextBlock(nextFarBlock);
        }

        // next unvisited offsets
        if (nextFarBlock)
            m_graph->getCallbacks()->onUnvisitedOffsetFound(nextFarBlock->getMinOffset());
        if (nextNearBlock)
            m_graph->getCallbacks()->onUnvisitedOffsetFound(nextNearBlock->getMinOffset());
    }
    else if (instruction->getId() != InstructionId::RETURN && instruction->getId() != InstructionId::INT) {
        if (auto nextBlock = m_graph->getBlockAt(nextOffset)) {
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
                    m_graph->joinBlocks(block, nextBlock);
                } else {
                    // otherwise, make a link
                    block->setNearNextBlock(nextBlock);
                }
            }
        }

        // next unvisited offset
        m_graph->getCallbacks()->onUnvisitedOffsetFound(nextOffset);
    }
}

void GraphConsistency::onInstructionRemoved(const Instruction* instruction)
{
    // if (instruction->isBranching()) {
    //     if (auto block = getBlockAt(offset)) {
    //         if (instruction == block->getInstructions().rbegin()->second) {
    //             block->setNearNextBlock(nullptr);
    //             block->setFarNextBlock(nullptr);
    //             block->update();
    //         }
    //     }
    // }
    // else if (instruction->getId() == pcode::InstructionId::CALL) {
    //     if (auto block = getBlockAt(offset)) {
    //         auto otherFunctionGraph = block->getFunctionGraph()->getReferencedGraphsFrom().at(offset);
    //         block->getFunctionGraph()->removeReferencedGraphFrom(offset);
    //         if (otherFunctionGraph->getReferencedGraphsTo().empty())
    //             removeFunctionGraph(otherFunctionGraph);
    //     }
    // }
}

void GraphConsistency::onBlockCreated(Block* block)
{}

void GraphConsistency::onBlockRemoved(Block* block)
{}

void GraphConsistency::onFunctionGraphCreated(FunctionGraph* functionGraph)
{}

void GraphConsistency::onFunctionGraphRemoved(FunctionGraph* functionGraph)
{}

InstructionOffset GraphConsistency::getTargetOffset(const pcode::Instruction* instr)
{
    InstructionOffset targetOffset = InvalidOffset;
    if (const auto constVarnode = std::dynamic_pointer_cast<pcode::ConstantVarnode>(instr->getInput0())) {
        // if this input contains a hardcoded constant
        targetOffset = constVarnode->getValue();
    }
    // check if the target offset is valid
    if (targetOffset == InvalidOffset || !m_graph->getInstructionProvider()->isOffsetValid(targetOffset.byteOffset)) {
        std::stringstream ss;
        ss << "Invalid target 0x" << utils::to_hex() << targetOffset << " at offset 0x" << utils::to_hex() << instr->getOffset();
        m_graph->getCallbacks()->onWarningEmitted(ss.str());
        targetOffset = InvalidOffset;
    }
    return targetOffset;
}