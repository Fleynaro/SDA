#include "SDA/Core/Pcode/PcodeBlock.h"
#include "SDA/Core/Pcode/PcodeGraph.h"

using namespace sda::pcode;

Block::Block(InstructionOffset minOffset)
    : m_minOffset(minOffset), m_maxOffset(minOffset)
{}

std::map<InstructionOffset, const Instruction*>& Block::getInstructions() {
    return m_instructions;
}

void Block::setNearNextBlock(Block* nearNextBlock) {
    nearNextBlock->m_referencedBlocks.remove(m_nearNextBlock);
    nearNextBlock->m_referencedBlocks.push_back(nearNextBlock);
    m_nearNextBlock = nearNextBlock;
}

Block* Block::getNearNextBlock() const {
    return m_nearNextBlock;
}

void Block::setFarNextBlock(Block* farNextBlock) {
    farNextBlock->m_referencedBlocks.remove(m_farNextBlock);
    farNextBlock->m_referencedBlocks.push_back(farNextBlock);
    m_farNextBlock = farNextBlock;
}

Block* Block::getFarNextBlock() const {
    return m_farNextBlock;
}

const std::list<Block*>& Block::getReferencedBlocks() const {
    return m_referencedBlocks;
}

InstructionOffset Block::getMinOffset() const {
    return m_minOffset;
}

void Block::setMaxOffset(InstructionOffset maxOffset) {
    m_maxOffset = maxOffset;
}

InstructionOffset Block::getMaxOffset() const {
    return m_maxOffset;
}

FunctionGraph* Block::getFunctionGraph() const {
    return m_functionGraph;
}

size_t Block::getLevel() const {
    return m_level;
}

bool Block::contains(InstructionOffset offset, bool halfInterval) const {
	return offset >= m_minOffset &&
        (halfInterval ? offset < m_maxOffset : offset <= m_maxOffset);
}

void Block::update() {
    if (m_functionGraph) {
        std::list<Block*> path;
        m_functionGraph->getEntryBlock()->update(path, m_functionGraph);
    }
}

void Block::update(std::list<Block*>& path, FunctionGraph* funcGraph) {
    //check if there's a loop
	for (auto it = path.rbegin(); it != path.rend(); it++) {
		if (this == *it) {
			return;
		}
	}

    path.push_back(this);
	m_level = std::max(m_level, path.size());
    m_functionGraph = funcGraph;
    if (m_nearNextBlock)
        m_nearNextBlock->update(path, funcGraph);
    if (m_farNextBlock)
        m_farNextBlock->update(path, funcGraph);
	path.pop_back();
}