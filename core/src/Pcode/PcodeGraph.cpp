#include "SDA/Core/Pcode/PcodeGraph.h"
#include <stdexcept>

using namespace sda::pcode;

Graph::Graph() {
    m_callbacks = std::make_unique<Callbacks>();
}

void Graph::addInstruction(const Instruction& instruction) {
    m_instructions[instruction.getOffset()] = instruction;
}

void Graph::removeInstruction(const Instruction* instruction) {
    auto offset = instruction->getOffset();
    auto it = m_instructions.find(offset);
    if (it == m_instructions.end())
        throw std::runtime_error("Instruction not found");

    if (instruction->isBranching()) {
        if (auto block = getBlockAt(offset)) {
            if (instruction == block->getInstructions().rbegin()->second) {
                block->setNearNextBlock(nullptr);
                block->setFarNextBlock(nullptr);
                block->update();
            }
        }
    }
    else if (instruction->getId() == pcode::InstructionId::CALL) {
        if (auto block = getBlockAt(offset)) {
            auto otherFunctionGraph = block->getFunctionGraph()->getReferencedGraphsFrom().at(offset);
            block->getFunctionGraph()->removeReferencedGraphFrom(offset);
            if (otherFunctionGraph->getReferencedGraphsTo().empty())
                removeFunctionGraph(otherFunctionGraph);
        }
    }

    m_instructions.erase(it);
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

void Graph::removeBlock(Block* block) {
    // remove all instructions from the block
    for (const auto& [_, instr] : block->getInstructions())
        removeInstruction(instr);
    
    if (block->getReferencedBlocks().size() == 0) {
        // remove the block from the graph
        m_blocks.erase(block->getMinOffset());
    } else {
        // resize the block to 0 (to remain references to it)
        block->setMaxOffset(block->getMinOffset());
    }
}

Block* Graph::splitBlock(Block* block, InstructionOffset offset) {
    if (offset <= block->getMinOffset() || offset >= block->getMaxOffset())
        throw std::runtime_error("Invalid offset");
    
    // create the new second block
    auto block1 = block;
    auto block2 = createBlock(offset);

    // split instructions
    std::map<InstructionOffset, const Instruction*> instrOfBlock1;
    std::map<InstructionOffset, const Instruction*> instrOfBlock2;
    for (const auto& pair : block1->getInstructions()) {
        if (pair.first < offset)
            instrOfBlock1.insert(pair);
        else instrOfBlock2.insert(pair);
    }
    block1->getInstructions() = instrOfBlock1;
    block2->getInstructions() = instrOfBlock2;

    // resize the blocks
    block1->setMaxOffset(offset);
    block2->setMaxOffset(block1->getMaxOffset());

    // reconnect the first block
    if (block1->getNearNextBlock())
        block2->setNearNextBlock(block1->getNearNextBlock());
    if (block1->getFarNextBlock())
        block2->setFarNextBlock(block1->getFarNextBlock());
    block1->setNearNextBlock(block2);
    block1->setFarNextBlock(nullptr);

    return block2;
}

void Graph::joinBlocks(Block* block1, Block* block2) {
    if (block1->getMaxOffset() != block2->getMinOffset())
        throw std::runtime_error("Blocks are not adjacent");
    auto refBlocksCount = block2->getReferencedBlocks().size();
    if (refBlocksCount >= (block1->getNearNextBlock() == block2 ? 2 : 1))
        throw std::runtime_error("Second block is referenced by other blocks");

    // join the instructions
    block1->getInstructions().insert(
        block2->getInstructions().begin(), block2->getInstructions().end());

    // resize the block
    block1->setMaxOffset(block2->getMaxOffset());

    // reconnect the blocks
    if (block2->getNearNextBlock())
        block1->setNearNextBlock(block2->getNearNextBlock());
    if (block2->getFarNextBlock())
        block1->setFarNextBlock(block2->getFarNextBlock());
    block2->setNearNextBlock(nullptr);
    block2->setFarNextBlock(nullptr);

    // remove the second block
    m_blocks.erase(block2->getMinOffset());
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

FunctionGraph* Graph::createFunctionGraph(Block* entryBlock) {
    auto offset = entryBlock->getMinOffset();
    if (m_functionGraphs.find(offset) != m_functionGraphs.end())
        throw std::runtime_error("Function graph already exists");

    m_functionGraphs[offset] = FunctionGraph(entryBlock);
    auto functionGraph = &m_functionGraphs[offset];
    
    getCallbacks()->onFunctionGraphCreated(functionGraph);

    return functionGraph;
}

void Graph::removeFunctionGraph(FunctionGraph* functionGraph) {
    auto offset = functionGraph->getEntryBlock()->getMinOffset();
    auto it = m_functionGraphs.find(offset);
    if (it == m_functionGraphs.end())
        throw std::runtime_error("Function graph not found");

    getCallbacks()->onFunctionGraphRemoved(functionGraph);

    std::list<Block*> path;
    functionGraph->getEntryBlock()->update(path, nullptr);
    m_functionGraphs.erase(it);
}

void Graph::setCallbacks(std::shared_ptr<Callbacks> callbacks) {
    m_callbacks = callbacks;
}

std::shared_ptr<Graph::Callbacks> Graph::getCallbacks() const {
    return m_callbacks;
}