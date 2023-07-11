#include "SDA/Core/IRcode/IRcodeBlock.h"
#include "SDA/Core/IRcode/IRcodeFunction.h"
#include "SDA/Core/IRcode/IRcodeProgram.h"
#include "SDA/Core/IRcode/IRcodeGenerator.h"
#include "SDA/Core/IRcode/IRcodeEvents.h"
#include "SDA/Core/Pcode/PcodeGraph.h"

using namespace sda;
using namespace sda::ircode;

Block::Block(pcode::Block* pcodeBlock, Function* function)
    : m_pcodeBlock(pcodeBlock), m_function(function)
{}

Function* Block::getFunction() const {
    return m_function;
}

Hash Block::getHash() const {
    return m_hash;
}

pcode::Block* Block::getPcodeBlock() const {
    return m_pcodeBlock;
}

std::string Block::getName() const {
    return m_pcodeBlock->getName();
}

size_t Block::getIndex() const {
    return m_pcodeBlock->getIndex();
}

std::list<std::unique_ptr<Operation>>& Block::getOperations() {
    return m_operations;
}

Operation* Block::getOperationAt(pcode::InstructionOffset offset) const {
    for (auto& op : m_operations) {
        for (auto pcodeOp : op->getPcodeInstructions()) {
            if (pcodeOp->getOffset() == offset) {
                return op.get();
            }
        }
    }
    return nullptr;
}

MemorySpace* Block::getMemorySpace() {
    return &m_memSpace;
}

Block* Block::getNearNextBlock() const {
    auto pcodeBlock = m_pcodeBlock->getNearNextBlock();
    if (!pcodeBlock || !pcodeBlock->hasSameEntryBlockAs(m_pcodeBlock)) { // we are isolated from other function graphs
        return nullptr;
    }
    return m_function->toBlock(pcodeBlock);
}

Block* Block::getFarNextBlock() const {
    auto pcodeBlock = m_pcodeBlock->getFarNextBlock();
    if (!pcodeBlock || !pcodeBlock->hasSameEntryBlockAs(m_pcodeBlock)) { // we are isolated from other function graphs
        return nullptr;
    }
    return m_function->toBlock(pcodeBlock);
}

std::list<Block*> Block::getReferencedBlocks() const {
    std::list<Block*> referencedBlocks;
    for (auto pcodeBlock : m_pcodeBlock->getReferencedBlocks()) {
        if (!pcodeBlock->hasSameEntryBlockAs(m_pcodeBlock)) { // we are isolated from other function graphs
            continue;
        }
        referencedBlocks.push_back(m_function->toBlock(pcodeBlock));
    }
    return referencedBlocks;
}

std::list<Block*> Block::getDominantBlocks() const {
    std::list<Block*> dominantBlocks;
    for (auto pcodeBlock : m_pcodeBlock->getDominantBlocks()) {
        dominantBlocks.push_back(m_function->toBlock(pcodeBlock));
    }
    return dominantBlocks;
}

std::shared_ptr<Value>& Block::getCondition() {
    return m_condition;
}

void Block::clear() {
    m_condition = nullptr;
    for (auto& op : m_operations) {
        m_function->getProgram()->getEventPipe()->send(
            OperationRemovedEvent(op.get()));
    }
    m_dominantHash = 0;
    m_memSpace.clear();
    clearVarIds();
}

void Block::passDescendants(std::function<void(Block* block, bool& goNextBlocks)> callback) {
    m_pcodeBlock->passDescendants([this, &callback](pcode::Block* pcodeBlock, bool& goNextBlocks) {
        if (!pcodeBlock->hasSameEntryBlockAs(m_pcodeBlock)) { // we are isolated from other function graphs
            return;
        }
        callback(m_function->toBlock(pcodeBlock), goNextBlocks);
    });
}

void Block::update() {
    // clear all descendant blocks
    utils::BitSet clearedBlocks;
    std::list<Block*> decompiledBlocks;
    passDescendants([&](Block* block, bool& goNextBlocks) {
        if (clearedBlocks.get(block->getIndex()))
            return;
        block->clear();
        clearedBlocks.set(block->getIndex(), true);
        decompiledBlocks.push_back(block);
        goNextBlocks = true;
    });
    // decompile all descendant blocks
    passDescendants([&](Block* block, bool& goNextBlocks) {
        block->decompile(goNextBlocks);
    });
    m_function->getProgram()->getEventPipe()->send(
        FunctionDecompiledEvent(m_function, std::move(decompiledBlocks)));
}

void Block::decompile(bool& goNextBlocks) {
    // if parent blocks have been changed, decompile this block again
    // "changes" can be checked by dominant hash
    auto actualDominantHash = calcDominantHash();
    if (actualDominantHash == m_dominantHash) {
        return;
    }
    Block tempBlock(m_pcodeBlock, m_function);
    auto nextVarIdProvider = [this]() {
        return getNextVarId();
    };
    IRcodeGenerator ircodeGen(&tempBlock, nullptr, nextVarIdProvider);
    auto& instructions = m_pcodeBlock->getInstructions();
    clearVarIds();
    for (auto& [offset, instruction] : instructions) {
        ircodeGen.ingestPcode(instruction);
    }
    replaceWith(&tempBlock);
    m_dominantHash = actualDominantHash;
    goNextBlocks = true;
}

Hash Block::calcHash() {
    // need as to be able to calculate dominant hash
    Hash hash = 0;
    for (auto& operation : m_operations) {
        boost::hash_combine(hash, operation->getHash());
    }
    return hash;
}

Hash Block::calcDominantHash() {
    // dominant hash is needed to check if parent blocks have been changed
    Hash hash = 0;
    for (auto block : getDominantBlocks()) {
        boost::hash_combine(hash, block->getHash());
    }
    return hash;
}

size_t Block::getNextVarId() {
    size_t freeIdx = 0;
    while (m_function->m_varIds.get(freeIdx)) {
        freeIdx++;
    }
    m_function->m_varIds.set(freeIdx, true);
    m_varIds.set(freeIdx, true);
    return freeIdx + 1;
}

void Block::clearVarIds() {
    m_function->m_varIds = m_function->m_varIds & ~m_varIds;
    m_varIds.clear();
}

void Block::replaceWith(Block* block) {
    // gather all related ref operations
    std::list<RefOperation*> refOperations;
    for (auto& oldOp : m_operations) {
        for (auto refOp : oldOp->getOutput()->getRefOperations()) {
            // ignore ref operations that are in the current block as they will be replaced
            bool isFound = false;
            for (auto& oldOp2 : m_operations) {
                if (refOp == oldOp2.get()) {
                    isFound = true;
                    break;
                }
            }
            if (!isFound) {
                refOp->clear();
                refOperations.push_back(refOp);
            }
        }
        m_function->getProgram()->getEventPipe()->send(
            OperationRemovedEvent(oldOp.get()));
    }
    // replace data
    m_memSpace = std::move(block->m_memSpace);
    m_condition = std::move(block->m_condition);
    m_operations = std::move(block->m_operations);
    for (auto& op : m_operations) {
        op->setBlock(this);
        m_function->getProgram()->getEventPipe()->send(
            OperationAddedEvent(op.get()));
    }
    m_hash = calcHash();
    // update ref operations
    for (auto refOp : refOperations) {
        auto& reference = refOp->getReference();
        auto foundTargetVariable = reference.block->getMemorySpace()->findVariable(reference);
        assert(foundTargetVariable);
        refOp->setTargetVariable(foundTargetVariable);
    }
}
