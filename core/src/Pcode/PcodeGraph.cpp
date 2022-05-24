#include "Core/Pcode/PcodeGraph.h"
#include <stdexcept>

using namespace sda::pcode;

void Graph::addInstruction(const Instruction& instruction) {
    m_instructions[instruction.getOffset()] = instruction;
}

const Instruction* Graph::getInstructionAt(InstructionOffset offset) const {
    auto it = m_instructions.find(offset);
    if (it == m_instructions.end()) {
        return nullptr;
    }
    return &it->second;
}

Block* Graph::createBlock(InstructionOffset offset) {
    if (m_blocks.find(offset) != m_blocks.end())
        throw std::runtime_error("Block already exists");
    m_blocks[offset] = Block(offset);
    return &m_blocks[offset];
}

void Graph::cleanBlock(Block* block) {
    // remove all instructions from the block
    for (auto instr : block->getInstructions())
        removeInstruction(instr);
    block->getInstructions().clear();
    
    // disconnect the block from the graph
    block->setNearNextBlock(nullptr);
    block->setFarNextBlock(nullptr);

    // resize the block to 0
    block->setMaxOffset(block->getMinOffset());
}

Block* Graph::getBlockAt(InstructionOffset offset, bool halfInterval) {
    if (!m_blocks.empty()) {
		const auto it = std::prev(m_blocks.upper_bound(offset));
		if (it != m_blocks.end()) {
			auto& pcodeBlock = it->second;
			if (pcodeBlock.contains(offset, halfInterval)) {
				return &pcodeBlock;
			}
		}
	}
	return nullptr;
}

void Graph::removeInstruction(const Instruction* instruction) {
    auto it = m_instructions.find(instruction->getOffset());
    if (it == m_instructions.end())
        throw std::runtime_error("Instruction not found");
    m_instructions.erase(it);
}