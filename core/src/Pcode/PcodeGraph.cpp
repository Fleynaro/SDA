#include "SDA/Core/Pcode/PcodeGraph.h"
#include <set>
#include <stdexcept>

using namespace sda::pcode;

InstructionOffset GetTargetOffset(const Instruction* instr) {
    InstructionOffset targetOffset = sda::InvalidOffset;
    if (const auto constVarnode = std::dynamic_pointer_cast<ConstantVarnode>(instr->getInput0())) {
        // if this input contains a hardcoded constant
        targetOffset = constVarnode->getValue();
    }
    return targetOffset;
}

Graph::Graph()
    : m_callbacks(std::make_unique<Callbacks>())
{}

void Graph::explore(InstructionOffset startOffset, InstructionProvider* instrProvider) {
    std::list<InstructionOffset> unvisitedOffsets = { startOffset };
    std::set<InstructionOffset> visitedOffsets;

    // set the callbacks to gather unvisited offsets
    struct ExploreCallbacks : Callbacks {
        std::list<InstructionOffset>* unvisitedOffsets;

        // void onBlockCreatedImpl(Block* block) override {
        //     //if (startOffset == block->getMinOffset())
        // }

        void onUnvisitedOffsetFoundImpl(InstructionOffset offset) override {
            unvisitedOffsets->push_back(offset);
        }
    };
    auto prevCallbacks = getCallbacks();
    auto exploreCallbacks = std::make_shared<ExploreCallbacks>();
    exploreCallbacks->unvisitedOffsets = &unvisitedOffsets;
    exploreCallbacks->setPrevCallbacks(prevCallbacks);
    setCallbacks(exploreCallbacks);

    while (!unvisitedOffsets.empty()) {
        // get the next unvisited offset
        const auto offset = unvisitedOffsets.front();
        unvisitedOffsets.pop_front();

        // if the offset is already visited, skip it, else mark it as visited
        if (visitedOffsets.find(offset) != visitedOffsets.end())
            continue;
        visitedOffsets.insert(offset);

        if (offset.index == 0) {
            // if there are instructions at this offset, don't visit it again (offset might not be in m_visitedOffsets!)
            if (getInstructionAt(offset))
                continue;

            const auto byteOffset = offset.byteOffset;
            if (!instrProvider->isOffsetValid(byteOffset))
                continue;
            size_t origInstrLength;
            std::list<Instruction> instructions;
            instrProvider->decode(byteOffset, instructions, origInstrLength);
            if (!instructions.empty()) {
                auto offsetAfterOrigInstr = InstructionOffset(byteOffset + origInstrLength, 0);
                unvisitedOffsets.push_back(offsetAfterOrigInstr);
                for (auto it = instructions.begin(); it != instructions.end(); ++it) {
                    InstructionOffset nextOffset;
                    if (it != std::prev(instructions.end())) {
                        nextOffset = InstructionOffset(it->getOffset() + 1);
                    } else {
                        nextOffset = offsetAfterOrigInstr;
                    }
                    addInstruction(*it, nextOffset);
                }
            }
        }
    }
    
    // restore the previous callbacks
    setCallbacks(prevCallbacks);
}


void Graph::addInstruction(const Instruction& instruction, InstructionOffset nextOffset) {
    auto offset = instruction.getOffset();
    if (m_instructions.find(offset) != m_instructions.end())
        throw std::runtime_error("Instruction already exists at this offset");
    m_instructions[offset] = instruction;

    // get or create a block at this offset
    auto block = getBlockAt(offset, false);
    if (!block) {
        block = createBlock(offset);
    }
    block->getInstructions()[offset] = &m_instructions[offset];
    if (block->getMaxOffset() < nextOffset)
        block->setMaxOffset(nextOffset);
    
    // handle each type of instruction
    if (instruction.isBranching()) {
        auto targetOffset = GetTargetOffset(&instruction);
        if (targetOffset == InvalidOffset)
            return;

        // near block
        pcode::Block* nextNearBlock = nullptr;
        if (instruction.getId() == pcode::InstructionId::CBRANCH) {
            if (!getBlockAt(nextOffset)) {
                nextNearBlock = createBlock(nextOffset);
                block->setNearNextBlock(nextNearBlock);
            }
        }

        // far block
        pcode::Block* nextFarBlock = nullptr;
        if (auto alreadyExistingBlock = getBlockAt(targetOffset)) {
            if (targetOffset == alreadyExistingBlock->getMinOffset()) {
                block->setFarNextBlock(nextFarBlock = alreadyExistingBlock);
            }
            else {
                nextFarBlock = splitBlock(alreadyExistingBlock, targetOffset);
                block->setFarNextBlock(nextFarBlock);
            }
        }
        else {
            nextFarBlock = createBlock(targetOffset);
            block->setFarNextBlock(nextFarBlock);
        }

        // next unvisited offsets
        if (nextFarBlock)
            getCallbacks()->onUnvisitedOffsetFound(nextFarBlock->getMinOffset());
        if (nextNearBlock)
            getCallbacks()->onUnvisitedOffsetFound(nextNearBlock->getMinOffset());
    }
    else if (instruction.getId() != InstructionId::RETURN && instruction.getId() != InstructionId::INT) {
        if (auto nextBlock = getBlockAt(nextOffset)) {
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
                    joinBlocks(block, nextBlock);
                } else {
                    // otherwise, make a link
                    block->setNearNextBlock(nextBlock);
                }
            }
        }
    }
    getCallbacks()->onInstructionAdded(&m_instructions[offset], nextOffset);
}

void Graph::removeInstruction(const Instruction* instruction) {
    auto offset = instruction->getOffset();
    auto it = m_instructions.find(offset);
    if (it == m_instructions.end())
        throw std::runtime_error("Instruction not found");
    getCallbacks()->onInstructionRemoved(instruction);
    m_instructions.erase(it);
}

const Instruction* Graph::getInstructionAt(InstructionOffset offset) const {
    auto it = m_instructions.find(offset);
    if (it == m_instructions.end()) {
        return nullptr;
    }
    return &it->second;
}

std::vector<const Instruction*> Graph::getInstructionsAt(Offset byteOffset) const {
    std::vector<const Instruction*> instructions;
    auto instrOffset = InstructionOffset(byteOffset, 0);
    while (instrOffset.index <= InstructionOffset::GetMaxIndex()) {
        auto instr = getInstructionAt(instrOffset);
        if (!instr)
            break;
        instructions.push_back(instr);
        instrOffset.index++;
    }
    return instructions;
}

Block* Graph::createBlock(InstructionOffset offset) {
    if (m_blocks.find(offset) != m_blocks.end())
        throw std::runtime_error("Block already exists");

    m_blocks[offset] = Block(this, offset);
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

FunctionGraph* Graph::getFunctionGraphAt(InstructionOffset offset) {
    auto it = m_functionGraphs.find(offset);
    if (it == m_functionGraphs.end())
        return nullptr;
    return &it->second;
}

FunctionGraph* Graph::createFunctionGraph(Block* entryBlock) {
    auto offset = entryBlock->getMinOffset();
    if (m_functionGraphs.find(offset) != m_functionGraphs.end())
        throw std::runtime_error("Function graph already exists");
    m_functionGraphs[offset] = FunctionGraph(entryBlock);
    entryBlock->update();
    auto functionGraph = &m_functionGraphs[offset];
    getCallbacks()->onFunctionGraphCreated(functionGraph);
    return functionGraph;
}

void Graph::removeFunctionGraph(FunctionGraph* functionGraph) {
    auto entryBlock = functionGraph->getEntryBlock();
    auto offset = entryBlock->getMinOffset();
    auto it = m_functionGraphs.find(offset);
    if (it == m_functionGraphs.end())
        throw std::runtime_error("Function graph not found");
    getCallbacks()->onFunctionGraphRemoved(functionGraph);
    entryBlock->update();
    m_functionGraphs.erase(it);
}

void Graph::setCallbacks(std::shared_ptr<Callbacks> callbacks) {
    m_callbacks = callbacks;
}

std::shared_ptr<Graph::Callbacks> Graph::getCallbacks() const {
    return m_callbacks;
}