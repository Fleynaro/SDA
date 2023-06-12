#include "SDA/Core/Pcode/PcodeGraph.h"
#include "SDA/Core/Pcode/PcodeEvents.h"
#include "SDA/Core/Commit.h"
#include <set>
#include <stdexcept>

using namespace sda::pcode;

InstructionOffset sda::pcode::GetTargetOffset(const Instruction* instr) {
    InstructionOffset targetOffset = sda::InvalidOffset;
    if (const auto constVarnode = std::dynamic_pointer_cast<ConstantVarnode>(instr->getInput0())) {
        // if this input contains a hardcoded constant
        targetOffset = constVarnode->getValue();
    }
    return targetOffset;
}

// class OptimizationCallbacks : public Graph::Callbacks {
//     // TODO: to use it we should move CALL references from the func. graph class to the block class
//     Graph* m_graph;
//     bool m_enabled = true;
//     std::list<Block*> m_blocksToUpdate;
//     std::set<Block*> m_updatedBlocks;

//     void onBlockUpdateRequestedImpl(Block* block) override {
//         if (!m_enabled) {
//             return;
//         }
//         m_blocksToUpdate.push_back(block);
//     }

//     void onBlockUpdatedImpl(Block* block) override {
//         if (!m_enabled) {
//             return;
//         }
//         m_updatedBlocks.insert(block);
//     }

//     void onCommitStartedImpl() override {
//         m_enabled = true;
//         m_graph->setUpdateBlocksEnabled(false);
//     }

//     void onCommitEndedImpl() override {
//         m_enabled = false;
//         m_graph->setUpdateBlocksEnabled(true);
//         for (auto block : m_blocksToUpdate) {
//             if (m_updatedBlocks.find(block) == m_updatedBlocks.end()) {
//                 block->update();
//             }
//         }
//         m_blocksToUpdate.clear();
//         m_updatedBlocks.clear();
//     }

// public:
//     OptimizationCallbacks(Graph* graph)
//         : m_graph(graph)
//     {}
// };

Graph::Graph(std::shared_ptr<EventPipe> eventPipe)
    : m_eventPipe(eventPipe)
{}

std::shared_ptr<sda::EventPipe> Graph::getEventPipe() {
    return m_eventPipe;
}

void Graph::explore(InstructionOffset startOffset, InstructionProvider* instrProvider) {
    CommitScope commit(m_eventPipe);
    std::list<InstructionOffset> unvisitedOffsets = { startOffset };
    std::set<InstructionOffset> visitedOffsets;

    auto unsubscribe = m_eventPipe->subscribe(std::function([&](const pcode::UnvisitedOffsetFoundEvent& event) {
        unvisitedOffsets.push_back(event.offset);
    }));

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
                auto& lastInstr = *instructions.rbegin();
                auto offsetAfterOrigInstr = InstructionOffset(byteOffset + origInstrLength, 0);
                if (!lastInstr.isNotGoingNext()) {
                    unvisitedOffsets.push_front(offsetAfterOrigInstr);
                }
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
    
    unsubscribe();
}


void Graph::addInstruction(const Instruction& instruction, InstructionOffset nextOffset) {
    CommitScope commit(m_eventPipe);
    auto offset = instruction.getOffset();
    if (m_instructions.find(offset) != m_instructions.end())
        throw std::runtime_error("Instruction already exists at this offset");
    m_instructions[offset] = instruction;

    // get or create a block at this offset
    auto block = getBlockAt(offset, false);
    if (block) {
        auto lastBlockInstr = block->getLastInstruction();
        if (lastBlockInstr && lastBlockInstr->isNotGoingNext()) {
            /*
                BRANCH <label> <--- end of the previous block
                NOP   <--- here we start exploring (we must not extend the previous block)
            */
            block = createBlock(offset);
        }
    } else {
        block = createBlock(offset);
    }
    block->getInstructions()[offset] = &m_instructions[offset];
    if (block->getMaxOffset() < nextOffset)
        block->setMaxOffset(nextOffset);
    
    // subscribe each type of instruction
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
                nextFarBlock = alreadyExistingBlock;
            }
            else {
                nextFarBlock = splitBlock(alreadyExistingBlock, targetOffset);
                if (block == alreadyExistingBlock) {
                    block = nextFarBlock;
                }
            }
        }
        else {
            nextFarBlock = createBlock(targetOffset);
        }
        block->setFarNextBlock(nextFarBlock);

        // next unvisited offsets
        if (nextFarBlock) {
            m_eventPipe->send(UnvisitedOffsetFoundEvent {
                nextFarBlock->getMinOffset()
            });
        }
        if (nextNearBlock) {
            m_eventPipe->send(UnvisitedOffsetFoundEvent {
                nextNearBlock->getMinOffset()
            });
        }
    }
    else if (instruction.getId() == pcode::InstructionId::CALL) {
        auto targetOffset = GetTargetOffset(&instruction);
        if (targetOffset != InvalidOffset) {
            pcode::Block* calledBlock = nullptr;
            if (auto alreadyExistingBlock = getBlockAt(targetOffset)) {
                if (targetOffset == alreadyExistingBlock->getMinOffset()) {
                    calledBlock = alreadyExistingBlock;
                }
                else {
                    // check test NewCallSplitBlock
                    calledBlock = splitBlock(alreadyExistingBlock, targetOffset);
                    if (block == alreadyExistingBlock) {
                        // check test CallSplitSameBlock
                        block = calledBlock;
                    }
                }
            }
            else {
                calledBlock = createBlock(targetOffset);
            }
            block->getFunctionGraph()->addReferenceFrom(offset, calledBlock);

            // next unvisited offsets
            if (calledBlock) {
                m_eventPipe->send(UnvisitedOffsetFoundEvent {
                    calledBlock->getMinOffset()
                });
            }
        }
    }

    if (!instruction.isNotGoingNext()) {
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
                if (block->canBeJoinedWith(nextBlock)) {
                    // check test JoinNextBlock
                    joinBlocks(block, nextBlock);
                } else {
                    // otherwise, make a link
                    block->setNearNextBlock(nextBlock);
                }
            }
        }
    }
    m_eventPipe->send(InstructionAddedEvent {
        &m_instructions[offset],
        nextOffset
    });
}

void Graph::removeInstruction(const Instruction* instruction) {
    auto offset = instruction->getOffset();
    auto it = m_instructions.find(offset);
    if (it == m_instructions.end())
        throw std::runtime_error("Instruction not found");
    m_eventPipe->send(InstructionRemovedEvent {
        instruction
    });
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
    auto newBlock = &m_blocks[offset];
    newBlock->update();
    m_eventPipe->send(BlockCreatedEvent {
        newBlock
    });
    return newBlock;
}

void Graph::removeBlock(Block* block) {
    auto functionGraph = block->getFunctionGraph();
    assert(block->getReferencedBlocks().empty());
    assert(functionGraph->getReferencesTo().empty() && functionGraph->getReferencesFrom().empty());
    // remove all instructions from the block
    for (const auto& [_, instr] : block->getInstructions())
        removeInstruction(instr);
    block->setNearNextBlock(nullptr);
    block->setFarNextBlock(nullptr);
    if (functionGraph) {
        removeFunctionGraph(functionGraph);
    }
    // remove the block from the graph
    m_blocks.erase(block->getMinOffset());
}

Block* Graph::splitBlock(Block* block, InstructionOffset offset) {
    CommitScope commit(m_eventPipe);
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
    block2->setMaxOffset(block1->getMaxOffset());
    block1->setMaxOffset(offset);

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
    CommitScope commit(m_eventPipe);
    assert(block1->canBeJoinedWith(block2));
    assert(block2->getFunctionGraph()->getEntryBlock() == block2);

    // save things before removing block 2
    auto savedInstructions = block2->getInstructions();
    block2->getInstructions().clear();
    auto savedMaxOffset = block2->getMaxOffset();
    auto savedNearNextBlock = block2->getNearNextBlock();
    auto savedFarNextBlock = block2->getFarNextBlock();
    if (savedFarNextBlock == block2) {
        savedFarNextBlock = block1;
    }
    // make block 2 isolated
    block2->setNearNextBlock(nullptr);
    block2->setFarNextBlock(nullptr);
    // move all CALL references
    auto funcGraph1 = block1->getFunctionGraph();
    auto funcGraph2 = block2->getFunctionGraph();
    funcGraph2->moveReferences(funcGraph1);
    // and then remove block 2 along with its function graph
    removeBlock(block2);

    // transfer all saved things from block 2 to block 1
    block1->getInstructions().insert(savedInstructions.begin(), savedInstructions.end());
    block1->setMaxOffset(savedMaxOffset);
    block1->setNearNextBlock(savedNearNextBlock);
    block1->setFarNextBlock(savedFarNextBlock);
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

Block* Graph::getBlockByName(const std::string& name) {
    for (auto& [_, block] : m_blocks) {
        if (block.getName() == name)
            return &block;
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
    m_eventPipe->send(FunctionGraphCreatedEvent {
        functionGraph
    });
    return functionGraph;
}

void Graph::removeFunctionGraph(FunctionGraph* functionGraph) {
    auto entryBlock = functionGraph->getEntryBlock();
    auto offset = entryBlock->getMinOffset();
    auto it = m_functionGraphs.find(offset);
    if (it == m_functionGraphs.end())
        throw std::runtime_error("Function graph not found");
    m_eventPipe->send(FunctionGraphRemovedEvent {
        functionGraph
    });
    entryBlock->update();
    it->second.removeAllReferences();
    m_functionGraphs.erase(it);
}

void Graph::setUpdateBlocksEnabled(bool enabled) {
    m_updateBlockState = enabled ? UpdateBlockState::Enabled : UpdateBlockState::Disabled;
}
