#include "SDA/Core/Pcode/PcodeStructurer.h"
#include <stdexcept>

using namespace sda::pcode;

StructBlock::StructBlock(Block* pcodeBlock)
    : m_pcodeBlock(pcodeBlock)
{}

std::string StructBlock::getName() {
    if (auto pcodeBlock = getBottom()->getPcodeBlock())
        return pcodeBlock->getName();
    return "unknown";
}

Block* StructBlock::getPcodeBlock() const {
    return m_pcodeBlock;
}

void StructBlock::setNearNextBlock(StructBlock* nearNextBlock) {
    m_nearNextBlock = nearNextBlock;
    nearNextBlock->m_referencedBlocks.push_back(this);
}

StructBlock* StructBlock::getNearNextBlock() const {
    return m_nearNextBlock;
}

void StructBlock::setFarNextBlock(StructBlock* farNextBlock) {
    m_farNextBlock = farNextBlock;
    farNextBlock->m_referencedBlocks.push_back(this);
}

StructBlock* StructBlock::getFarNextBlock() const {
    return m_farNextBlock;
}

std::list<StructBlock*> StructBlock::getNextBlocks() const {
    std::list<StructBlock*> nextBlocks;
    if (m_nearNextBlock) {
        nextBlocks.push_back(m_nearNextBlock);
    }
    if (m_farNextBlock) {
        nextBlocks.push_back(m_farNextBlock);
    }
    return nextBlocks;
}

void StructBlock::setGoto(StructBlock* gotoBlock, GotoType type) {
    m_goto = gotoBlock;
    m_gotoType = type;
    m_goto->m_gotoReferencedBlocks.push_back(this);
}

StructBlock* StructBlock::getGoto() const {
    return m_goto;
}

GotoType StructBlock::getGotoType() const {
    return m_gotoType;
}

const std::list<StructBlock*>& StructBlock::getGotoReferencedBlocks() const {
    return m_gotoReferencedBlocks;
}

void StructBlock::moveNextBlocksTo(StructBlock* block) {
    if (m_nearNextBlock) {
        m_nearNextBlock->m_referencedBlocks.remove(this);
        m_nearNextBlock = nullptr;
        if (block) {
            block->setNearNextBlock(m_nearNextBlock);
        }
    }
    if (m_farNextBlock) {
        m_farNextBlock->m_referencedBlocks.remove(this);
        m_farNextBlock = nullptr;
        if (block) {
            block->setFarNextBlock(m_farNextBlock);
        }
    }
}

const std::list<StructBlock*>& StructBlock::getReferencedBlocks() const {
    return m_referencedBlocks;
}

void StructBlock::moveReferencedBlocksTo(StructBlock* block) {
    for (auto refBlock : m_referencedBlocks) {
        if (refBlock->m_nearNextBlock == this) {
            refBlock->m_nearNextBlock = block;
        }
        if (refBlock->m_farNextBlock == this) {
            refBlock->m_farNextBlock = block;
        }
        if (block) {
            block->m_referencedBlocks.push_back(refBlock);
        }
    }
    m_referencedBlocks.clear();
}

void StructBlock::setParent(StructBlock* parent) {
    assert(!m_parent || !parent);
    m_parent = parent;
}

StructBlock* StructBlock::getParent() const {
    return m_parent;
}

StructBlock* StructBlock::getTop() {
    auto block = this;
    while (block && block->m_parent) {
        block = block->m_parent;
    }
    return block;
}

StructBlock* StructBlock::getBottom() {
    std::list<StructBlock*> leafs;
    getLeafs(leafs);
    return leafs.front();
}

void StructBlock::getLeafs(std::list<StructBlock*>& leafs) {
    leafs.push_back(this);
}

StructBlockSequence::StructBlockSequence()
    : StructBlock(nullptr)
{}

void StructBlockSequence::addBlock(StructBlock* block) {
    m_blocks.push_back(block);
}

const std::list<StructBlock*>& StructBlockSequence::getBlocks() const {
    return m_blocks;
}

void StructBlockSequence::getLeafs(std::list<StructBlock*>& leafs) {
    for (auto block : m_blocks) {
        block->getLeafs(leafs);
    }
}

void StructBlockSequence::embed() {
    if (m_embedded) return;
    m_embedded = true;
    for (auto block : m_blocks) {
        block->setParent(this);
        block->embed();
    }
}

StructBlockIf::StructBlockIf()
    : StructBlock(nullptr)
{}

void StructBlockIf::setCondBlock(StructBlock* condBlock) {
    m_condBlock = condBlock;
}

StructBlock* StructBlockIf::getCondBlock() const {
    return m_condBlock;
}

void StructBlockIf::setThenBlock(StructBlock* thenBlock) {
    m_thenBlock = thenBlock;
}

StructBlock* StructBlockIf::getThenBlock() const {
    return m_thenBlock;
}

void StructBlockIf::setElseBlock(StructBlock* elseBlock) {
    m_elseBlock = elseBlock;
}

StructBlock* StructBlockIf::getElseBlock() const {
    return m_elseBlock;
}

void StructBlockIf::setInverted(bool inverted) {
    m_inverted = inverted;
}

bool StructBlockIf::isInverted() const {
    return m_inverted;
}

void StructBlockIf::getLeafs(std::list<StructBlock*>& leafs) {
    if (m_condBlock) {
        m_condBlock->getLeafs(leafs);
    }
    if (m_thenBlock) {
        m_thenBlock->getLeafs(leafs);
    }
    if (m_elseBlock) {
        m_elseBlock->getLeafs(leafs);
    }
}

void StructBlockIf::embed() {
    if (m_embedded) return;
    m_embedded = true;
    if (m_condBlock) {
        m_condBlock->setParent(this);
        m_condBlock->embed();
    }
    if (m_thenBlock) {
        m_thenBlock->setParent(this);
        m_thenBlock->embed();
    }
    if (m_elseBlock) {
        m_elseBlock->setParent(this);
        m_elseBlock->embed();
    }
}

StructBlockWhile::StructBlockWhile()
    : StructBlock(nullptr)
{}

void StructBlockWhile::setBodyBlock(StructBlock* bodyBlock) {
    m_bodyBlock = bodyBlock;
}

StructBlock* StructBlockWhile::getBodyBlock() const {
    return m_bodyBlock;
}

void StructBlockWhile::getLeafs(std::list<StructBlock*>& leafs) {
    m_bodyBlock->getLeafs(leafs);
}

void StructBlockWhile::embed() {
    if (m_embedded) return;
    m_embedded = true;
    if (m_bodyBlock) {
        m_bodyBlock->setParent(this);
        m_bodyBlock->embed();
    }
}

StructTree::StructTree() {}

StructBlock* StructTree::getEntryBlock() const {
    return m_entryBlock;
}

void StructTree::addBlock(std::unique_ptr<StructBlock> block) {
    m_blocks.push_back(std::move(block));
}

void StructTree::clear() {
    m_blocks.clear();
}

Structurer::Structurer(FunctionGraph* funcGraph, StructTree* tree)
    : m_funcGraph(funcGraph), m_tree(tree)
{}

void Structurer::start() {
    initTree();

    // loops
    findLoops();
    for (auto loop : m_loops) {
        handleLoop(loop);
    }

    while(true) {
        std::list<Pattern> patterns;
        for (auto block : m_blocksToProcess) {
            auto recognizers = {
                TrySequencePattern,
                TryIfPattern,
                TryIfElsePattern,
                TryIfGotoPattern,
                TryWhilePattern,
                TryReplaceWithGotoPattern
            };
            bool exit = false;
            for (auto recognizer : recognizers) {
                Pattern pattern;
                if (recognizer(this, block, pattern)) {
                    patterns.push_back(std::move(pattern));
                    if (pattern.score == 0) {
                        exit = true;
                        break;
                    }
                }
            }
            if (exit) break;
        }
        if (patterns.empty())
            break;
        Pattern* patternToApply = nullptr;
        size_t minScore = -1;
        for (auto& pattern : patterns) {
            if (pattern.score < minScore) {
                minScore = pattern.score;
                patternToApply = &pattern;
            }
        }
        assert(patternToApply);
        applyPattern(*patternToApply);
    }
    
    if (m_blocksToProcess.size() >= 2) {
        // see test PcodeStructurerTest.ComplexGoto
        auto mainSequence = createSequenceBlock(m_blocksToProcess);
        mainSequence->embed();
        m_tree->m_entryBlock = mainSequence.get();
        m_tree->addBlock(std::move(mainSequence));
    } else {
        m_tree->m_entryBlock = m_blocksToProcess.front();
    }

    m_blocksToProcess.clear();
}

std::list<std::tuple<StructBlock*, StructBlock*, bool>> Combinations(const std::list<StructBlock*>& blocks) {
    if (blocks.size() == 0)
        return {};
    if (blocks.size() == 1)
        return { { blocks.front(), nullptr, false } };
    return {
        { blocks.front(), blocks.back(), false },
        { blocks.back(), blocks.front(), true }
    };
}

StructBlockIf* MakeGoto(StructBlock* destBlock, GotoType type, std::list<std::unique_ptr<StructBlock>>& newBlocks) {
    auto cond = std::make_unique<StructBlockIf>();
    auto gotoBlock = std::make_unique<StructBlock>();
    gotoBlock->setGoto(destBlock, type);
    cond->setThenBlock(gotoBlock.get());
    auto condPtr = cond.get();
    newBlocks.push_front(std::move(gotoBlock));
    newBlocks.push_front(std::move(cond));
    return condPtr;
}

bool Structurer::TrySequencePattern(Structurer* structurer, StructBlock* startBlock, Pattern& pattern) {
    // see test PcodeStructurerTest.Sequence
    auto nextBlocks = startBlock->getNextBlocks();
    if (nextBlocks.size() != 1) return false;
    auto nextBlock = nextBlocks.front();
    if (nextBlock->getReferencedBlocks().size() != 1) return false;
    if (auto loop = structurer->getLoopAt(nextBlock)) {
        auto block = loop->startBlock;
        while (block) {
            block = block->getParent();
            if (dynamic_cast<StructBlockWhile*>(block)) break;
        }
        if (!block) {
            return false;
        }
    }
    auto sequence = std::make_unique<StructBlockSequence>();
    sequence->addBlock(startBlock);
    sequence->addBlock(nextBlock);
    pattern.newBlocks.push_front(std::move(sequence));
    pattern.blocksToReplace = { startBlock, nextBlock };
    pattern.startBlock = startBlock;
    pattern.nearNextBlock = nextBlock->getNearNextBlock();
    pattern.farNextBlock = nextBlock->getFarNextBlock();
    pattern.score = 0;
    return true;
}

bool Structurer::TryIfPattern(Structurer* structurer, StructBlock* startBlock, Pattern& pattern) {
    // see test PcodeStructurerTest.If
    auto nextBlocks = startBlock->getNextBlocks();
    if (nextBlocks.size() != 2) return false;
    for (auto [endBlock, thenBlock, inverted] : Combinations(nextBlocks)) {
        if (thenBlock->getReferencedBlocks().size() != 1) continue;
        for (auto [endBlock_, otherBlock, otherInverted] : Combinations(thenBlock->getNextBlocks())) {
            if (endBlock != endBlock_) continue;
            auto cond = std::make_unique<StructBlockIf>();
            cond->setInverted(inverted);
            cond->setCondBlock(startBlock);
            if (otherBlock) {
                auto gotoBlock = MakeGoto(otherBlock, GotoType::Default, pattern.newBlocks);
                gotoBlock->setInverted(otherInverted);
                gotoBlock->setCondBlock(thenBlock);
                cond->setThenBlock(gotoBlock);
                pattern.score ++;
            } else {
                cond->setThenBlock(thenBlock);
            }
            pattern.newBlocks.push_front(std::move(cond));
            pattern.blocksToReplace = { startBlock, thenBlock };
            pattern.startBlock = startBlock;
            pattern.nearNextBlock = endBlock;
            return true;
        }
    }
    return false;
}

bool Structurer::TryIfElsePattern(Structurer* structurer, StructBlock* startBlock, Pattern& pattern) {
    // see test PcodeStructurerTest.IfElse
    auto nextBlocks = startBlock->getNextBlocks();
    if (nextBlocks.size() != 2) return false;
    auto [thenBlock, elseBlock] = std::make_pair(nextBlocks.front(), nextBlocks.back());
    if (thenBlock->getReferencedBlocks().size() != 1) return false;
    if (elseBlock->getReferencedBlocks().size() != 1) return false;
    for (auto [endBlock1, thenOtherBlock, thenOtherInverted] : Combinations(thenBlock->getNextBlocks())) {
        for (auto [endBlock2, elseOtherBlock, elseOtherInverted] : Combinations(elseBlock->getNextBlocks())) {
            if (endBlock1 != endBlock2) continue;
            auto cond = std::make_unique<StructBlockIf>();
            cond->setInverted(true);
            cond->setCondBlock(startBlock);
            pattern.score = 0;
            if (thenOtherBlock) {
                auto gotoBlock = MakeGoto(thenOtherBlock, GotoType::Default, pattern.newBlocks);
                gotoBlock->setInverted(thenOtherInverted);
                gotoBlock->setCondBlock(thenBlock);
                cond->setThenBlock(gotoBlock);
                pattern.score ++;
            } else {
                cond->setThenBlock(thenBlock);
            }
            if (elseOtherBlock) {
                auto gotoBlock = MakeGoto(elseOtherBlock, GotoType::Default, pattern.newBlocks);
                gotoBlock->setInverted(elseOtherInverted);
                gotoBlock->setCondBlock(elseBlock);
                cond->setElseBlock(gotoBlock);
                pattern.score ++;
            } else {
                cond->setElseBlock(elseBlock);
            }
            pattern.newBlocks.push_front(std::move(cond));
            pattern.blocksToReplace = { startBlock, thenBlock, elseBlock };
            pattern.startBlock = startBlock;
            pattern.nearNextBlock = endBlock1;
            pattern.score = 0;
            return true;
        }
    }
    return false;
}

bool Structurer::TryIfGotoPattern(Structurer* structurer, StructBlock* startBlock, Pattern& pattern) {
    // see test PcodeStructurerTest.GotoToWhile
    auto nextBlocks = startBlock->getNextBlocks();
    if (nextBlocks.size() != 2) return false;
    auto ifBlock = MakeGoto(nextBlocks.back(), GotoType::Default, pattern.newBlocks);
    ifBlock->setCondBlock(startBlock);
    pattern.blocksToReplace = { startBlock };
    pattern.startBlock = startBlock;
    pattern.nearNextBlock = nextBlocks.front();
    pattern.score = 10;
    return true;
}

bool Structurer::TryReplaceWithGotoPattern(Structurer* structurer, StructBlock* startBlock, Pattern& pattern) {
    // see test PcodeStructurerTest.ComplexGoto
    if (!startBlock->getReferencedBlocks().empty()) return false;
    auto nextBlocks = startBlock->getNextBlocks();
    if (nextBlocks.size() != 1) return false;
    auto nextBlock = nextBlocks.front();
    pattern.customProcessing = [startBlock, nextBlock]() {
        startBlock->moveNextBlocksTo(nullptr);
        startBlock->setGoto(nextBlock, GotoType::Default);
    };
    pattern.score = 1;
    return true;
}

bool Structurer::TryWhilePattern(Structurer* structurer, StructBlock* startBlock, Pattern& pattern) {
    // see test PcodeStructurerTest.WhileNoCond
    auto nextBlocks = startBlock->getNextBlocks();
    if (nextBlocks.size() != 1) return false;
    auto nextBlock = nextBlocks.front();
    if (startBlock != nextBlock) return false;
    auto loopInfo = structurer->getLoopAt(startBlock);
    assert(loopInfo);
    auto loop = std::make_unique<StructBlockWhile>();
    loop->setBodyBlock(startBlock);
    pattern.newBlocks.push_front(std::move(loop));
    pattern.nearNextBlock = loopInfo->exitBlock->getTop();
    pattern.blocksToReplace = { startBlock };
    pattern.startBlock = startBlock;
    pattern.score = 0;
    return true;
}

void Structurer::applyPattern(Pattern& pattern) {
    // custom logic to apply
    if (pattern.customProcessing) {
        assert(pattern.newBlocks.empty());
        pattern.customProcessing();
        return;
    }

    // embed
    auto newRootBlock = pattern.newBlocks.front().get();
    newRootBlock->embed();
    for (auto& block : pattern.newBlocks) {
        m_tree->addBlock(std::move(block));
    }

    // connections
    pattern.startBlock->moveReferencedBlocksTo(newRootBlock);
    for (auto block : pattern.blocksToReplace) {
        block->moveReferencedBlocksTo(nullptr);
        block->moveNextBlocksTo(nullptr);
    }
    if (pattern.nearNextBlock) {
        auto nearNextBlock = pattern.nearNextBlock->getTop();
        newRootBlock->setNearNextBlock(nearNextBlock);
    }
    if (pattern.farNextBlock) {
        auto farNextBlock = pattern.farNextBlock->getTop();
        newRootBlock->setFarNextBlock(farNextBlock);
    }

    // add to process list
    auto it = std::find(m_blocksToProcess.begin(), m_blocksToProcess.end(), pattern.startBlock);
    assert(it != m_blocksToProcess.end());
    m_blocksToProcess.insert(it, newRootBlock);
    for (auto block : pattern.blocksToReplace)
        m_blocksToProcess.remove(block);
}

void Structurer::initTree() {
    m_blockToInfo.clear();
    m_blocksToProcess.clear();
    m_dominants.clear();
    m_tree->clear();

    // make initial tree
    std::map<Block*, StructBlock*> pcodeBlockToStructBlock;
    for (auto info : m_funcGraph->getBlocks()) {
        auto structBlock = std::make_unique<StructBlock>(info.block);
        m_blockToInfo[structBlock.get()] = { m_blockToInfo.size(), info.level };
        pcodeBlockToStructBlock.insert({ info.block, structBlock.get() });
        m_blocksToProcess.push_back(structBlock.get());
        m_tree->addBlock(std::move(structBlock));
    }
    for (auto& [pcodeBlock, structBlock] : pcodeBlockToStructBlock) {
        if (auto nearNextBlock = pcodeBlock->getNearNextBlock()) {
            auto it = pcodeBlockToStructBlock.find(nearNextBlock);
            if (it != pcodeBlockToStructBlock.end()) {
                structBlock->setNearNextBlock(it->second);
            }
        }
        if (auto farNextBlock = pcodeBlock->getFarNextBlock()) {
            auto it = pcodeBlockToStructBlock.find(farNextBlock);
            if (it != pcodeBlockToStructBlock.end()) {
                structBlock->setFarNextBlock(it->second);
            }
        }
    }

    // calculate back level
    for (auto& [structBlock, _] : m_blockToInfo) {
        bool isEnd = true;
        for (auto nextBlock : structBlock->getNextBlocks()) {
            if (m_blockToInfo[structBlock].level < m_blockToInfo[nextBlock].level) {
                isEnd = false;
                break;
            }
        }
        if (isEnd) {
            std::list<StructBlock*> path;
            calculateBackLevel(structBlock, path);
        }
    }

    m_tree->m_entryBlock = pcodeBlockToStructBlock[m_funcGraph->getEntryBlock()];
    m_dominants = getDominants(m_tree->m_entryBlock);

    // sort blocks to process by dominant count
    m_blocksToProcess.sort([&](StructBlock* a, StructBlock* b) {
        // start with the nested block
        return m_dominants[a].size() > m_dominants[b].size();
    });
}

void Structurer::calculateBackLevel(StructBlock* block, std::list<StructBlock*>& path) {
    for (auto it = path.rbegin(); it != path.rend(); ++it) {
        if (*it == block) {
            return;
        }
    }
    path.push_back(block);
    for (auto refBlock : block->getReferencedBlocks()) {
        calculateBackLevel(refBlock, path);
    }
    auto& backLevel = m_blockToInfo[block].backLevel;
    backLevel = std::max(backLevel, path.size());
    path.pop_back();
}

void Structurer::findLoops() {
    m_loops.clear();
    m_blockToLoop.clear();

    // find all loops
    std::set<StructBlock*> startBlocks;
    for (auto& [structBlock, info] : m_blockToInfo) {
        if (auto farNextBlock = structBlock->getFarNextBlock()) {
            auto& farInfo = m_blockToInfo[farNextBlock];
            if (farInfo.level <= info.level) {
                if (startBlocks.find(farNextBlock) != startBlocks.end()) continue;
                startBlocks.insert(farNextBlock);
                LoopInfo loopInfo;
                loopInfo.startBlock = farNextBlock;
                m_loops.push_back(loopInfo);
            }
        }
    }

    // find last continue blocks
    for (auto& loopInfo : m_loops) {
        auto& info = m_blockToInfo[loopInfo.startBlock];
        StructBlock* lastContinueBlock = nullptr;
        size_t maxLevel = 0;
        for (auto refBlock : loopInfo.startBlock->getReferencedBlocks()) {
            auto refLevel = m_blockToInfo[refBlock].level;
            if (refLevel < info.level) continue;
            if (refLevel > maxLevel) {
                maxLevel = refLevel;
                lastContinueBlock = refBlock;
            }
        }
        assert(lastContinueBlock);
        loopInfo.lastContinueBlock = lastContinueBlock;
    }

    // find body blocks
    for (auto& loopInfo : m_loops) {
        std::map<StructBlock*, utils::BitSet> blockToParents;
        passDescendants(loopInfo.startBlock, [&](StructBlock* block, bool& goNextBlocks) {
            if (!loopInfo.bodyBlocks.empty()) return;

            utils::BitSet set;
            for (auto refBlock : getReferencedBlocks(block, true)) {
                auto it = blockToParents.find(refBlock);
                if (it == blockToParents.end())
                    continue;
                set = set | it->second;
            }
            set.set(m_blockToInfo[block].index, true);

            if (block == loopInfo.lastContinueBlock) {
                for (auto& [structBlock, info] : m_blockToInfo) {
                    if (!set.get(info.index)) continue;
                    loopInfo.bodyBlocks.insert(structBlock);
                }
                return;
            }

            blockToParents[block] = set;
            goNextBlocks = true;
        }, true);
    }

    // find exit block
    for (auto& loopInfo : m_loops) {
        // gather exit blocks
        std::set<StructBlock*> exitBlocks;
        for (auto bodyBlock : loopInfo.bodyBlocks) {
            auto nextBlocks = bodyBlock->getNextBlocks();
            if (nextBlocks.size() != 2) continue;
            for (auto nextBlock : nextBlocks) {
                if (loopInfo.bodyBlocks.find(nextBlock) == loopInfo.bodyBlocks.end()) {
                    exitBlocks.insert(nextBlock);
                }
            }
        }
        // select exit block with max back level
        if (!exitBlocks.empty()) {
            size_t maxBackLevel = 0;
            loopInfo.exitBlock = *exitBlocks.begin();
            for (auto exitBlock : exitBlocks) {
                auto backLevel = m_blockToInfo[exitBlock].backLevel;
                if (backLevel > maxBackLevel) {
                    maxBackLevel = backLevel;
                    loopInfo.exitBlock = exitBlock;
                }
            }
        }
    }

    // sort
    m_loops.sort([&](const LoopInfo& a, const LoopInfo& b) {
        // start with the nested loop
        return m_dominants[a.startBlock].size() > m_dominants[b.startBlock].size();
        //return m_blockToInfo[a.startBlock].level > m_blockToInfo[b.startBlock].level;
    });

    // set map
    for (auto& loopInfo : m_loops) {
        m_blockToLoop[loopInfo.startBlock] = &loopInfo;
    }
}

void Structurer::handleLoop(const LoopInfo& loopInfo) {
    // continue
    for (auto bodyBlock : loopInfo.bodyBlocks) {
        if (bodyBlock == loopInfo.lastContinueBlock) continue;
        for (auto& [startLoopBlock, nextBlock, inverted] : Combinations(bodyBlock->getNextBlocks())) {
            if (startLoopBlock == loopInfo.startBlock) {
                Pattern pattern;
                auto continueIfBlock = MakeGoto(startLoopBlock, GotoType::Continue, pattern.newBlocks);
                continueIfBlock->setInverted(!inverted);
                continueIfBlock->setCondBlock(bodyBlock);
                pattern.blocksToReplace = { bodyBlock };
                pattern.startBlock = bodyBlock;
                pattern.nearNextBlock = nextBlock;
                pattern.score = 0;
                applyPattern(pattern);
            }
        }
    }

    // break or goto
    for (auto bodyBlock : loopInfo.bodyBlocks) {
        for (auto& [exitBlock, nextBlock, inverted] : Combinations(bodyBlock->getNextBlocks())) {
            auto bottomExitBlock = exitBlock->getBottom();
            if (loopInfo.bodyBlocks.find(bottomExitBlock) != loopInfo.bodyBlocks.end()) continue;
            Pattern pattern;
            auto type = bottomExitBlock == loopInfo.exitBlock ? GotoType::Break : GotoType::Default;
            auto breakIfBlock = MakeGoto(exitBlock, type, pattern.newBlocks);
            breakIfBlock->setInverted(!inverted);
            breakIfBlock->setCondBlock(bodyBlock);
            pattern.blocksToReplace = { bodyBlock };
            pattern.startBlock = bodyBlock;
            pattern.nearNextBlock = nextBlock;
            pattern.score = 0;
            applyPattern(pattern);
        }
    }
}

Structurer::LoopInfo* Structurer::getLoopAt(StructBlock* block) {
    auto it = m_blockToLoop.find(block->getBottom());
    if (it != m_blockToLoop.end()) {
        return it->second;
    }
    return nullptr;
}

size_t Structurer::getMinLevel(StructBlock* block) {
    std::list<StructBlock*> leafs;
    block->getLeafs(leafs);
    size_t minLevel = -1;
    for (auto leaf : leafs) {
        auto it = m_blockToInfo.find(leaf);
        if (it != m_blockToInfo.end()) {
            if (it->second.level < minLevel) {
                minLevel = it->second.level;
            }
        }
    }
    return minLevel;
}

std::unique_ptr<StructBlockSequence> Structurer::createSequenceBlock(const std::list<StructBlock*>& blocks) {
    auto sequence = std::make_unique<StructBlockSequence>();
    std::list<std::pair<StructBlock*, size_t>> blocksWithLevel;
    for (auto block : blocks) {
        assert(block->getReferencedBlocks().empty() && block->getNextBlocks().empty());
        blocksWithLevel.emplace_back(block, getMinLevel(block));
    }
    blocksWithLevel.sort([](const auto& a, const auto& b) {
        return a.second < b.second;
    });
    for (auto& [block, _] : blocksWithLevel) {
        sequence->addBlock(block);
    }
    return sequence;
}

std::map<StructBlock*, utils::BitSet> Structurer::getDominants(StructBlock* block) {
    std::map<StructBlock*, utils::BitSet> blockToDominants;
    passDescendants(block, [&](StructBlock* block, bool& goNextBlocks) {
        utils::BitSet dominants;
        bool isFirst = true;
        for (auto parent : getReferencedBlocks(block, true)) {
            auto it = blockToDominants.find(parent);
            if (it == blockToDominants.end())
                continue;
            if (isFirst) {
                dominants = it->second;
                isFirst = false;
            } else {
                dominants = dominants & it->second;
            }
        }
        dominants.set(m_blockToInfo[block].index, true);
        goNextBlocks = !(blockToDominants[block] == dominants);
        blockToDominants[block] = dominants;
    }, true);
    return blockToDominants;
}

void Structurer::passDescendants(StructBlock* startBlock, std::function<void(StructBlock* block, bool& goNextBlocks)> callback, bool ignoreLoop) {
    std::map<StructBlock*, size_t> blockKnocks;
    std::list<StructBlock*> blocksToVisit;
    blocksToVisit.push_back(startBlock);
    do {
        while (!blocksToVisit.empty()) {
            auto block = blocksToVisit.front();
            blocksToVisit.pop_front();
            auto it = blockKnocks.find(block);
            if (it == blockKnocks.end()) {
                it = blockKnocks.insert({ block, 0 }).first;
            }
            auto knocks = ++it->second;
            if (knocks < getReferencedBlocks(block, ignoreLoop).size()) {
                continue;
            }
            blockKnocks.erase(it);
            bool goNextBlocks = false;
            callback(block, goNextBlocks);
            if (goNextBlocks) {
                for (auto nextBlock : getNextBlocks(block, ignoreLoop)) {
                    blocksToVisit.push_back(nextBlock);
                }
            }
        }
        if (!blockKnocks.empty()) {
            auto block = blockKnocks.begin()->first;
            blocksToVisit.push_back(block);
        }
    } while (!blocksToVisit.empty());
}

std::list<StructBlock*> Structurer::getReferencedBlocks(StructBlock* block, bool ignoreLoop) {
    if (!ignoreLoop)
        return block->getReferencedBlocks();
    std::list<StructBlock*> blocks;
    for (auto refBlock : block->getReferencedBlocks()) {
        if (m_blockToInfo[refBlock].level < m_blockToInfo[block].level) {
            blocks.push_back(refBlock);
        }
    }
    return blocks;
}

std::list<StructBlock*> Structurer::getNextBlocks(StructBlock* block, bool ignoreLoop) {
    if (!ignoreLoop)
        return block->getNextBlocks();
    std::list<StructBlock*> blocks;
    for (auto nextBlock : block->getNextBlocks()) {
        if (m_blockToInfo[nextBlock].level > m_blockToInfo[block].level) {
            blocks.push_back(nextBlock);
        }
    }
    return blocks;
}

StructTreePrinter::StructTreePrinter()
{}

void StructTreePrinter::setCodePrinter(PrinterFunction codePrinter) {
    m_codePrinter = codePrinter;
}

void StructTreePrinter::setConditionPrinter(PrinterFunction conditionPrinter) {
    m_conditionPrinter = conditionPrinter;
}

void StructTreePrinter::printStructTree(StructTree* tree) {
    printStructBlock(tree->getEntryBlock());
}

void StructTreePrinter::printStructBlock(StructBlock* block) {
    if (!block->getGotoReferencedBlocks().empty()) {
        bool hasDefaultGoto = false;
        for (auto gotoBlock : block->getGotoReferencedBlocks()) {
            if (gotoBlock->getGotoType() == GotoType::Default) {
                hasDefaultGoto = true;
                break;
            }
        }
        if (hasDefaultGoto) {
            printToken(std::string("label_") + block->getName(), IDENTIFIER);
            printToken(":", KEYWORD);
            newLine();
        }
    }
    bool isEmptyBlock = false;
    if (auto blockSequence = dynamic_cast<StructBlockSequence*>(block)) {
        printStructBlockSequence(blockSequence);
    } else if (auto blockIf = dynamic_cast<StructBlockIf*>(block)) {
        printStructBlockIf(blockIf);
    } else if (auto whileBlock = dynamic_cast<StructBlockWhile*>(block)) {
        printStructBlockWhile(whileBlock);
    } else {
        if (block->getPcodeBlock()) {
            m_codePrinter(block->getPcodeBlock());
        } else {
            isEmptyBlock = true;
        }
    }
    if (block->getGoto()) {
        if (!isEmptyBlock) newLine();
        switch (block->getGotoType()) {
        case GotoType::Default:
            printToken("goto ", KEYWORD);
            printToken(std::string("label_") + block->getGoto()->getName(), IDENTIFIER);
            printToken(";", KEYWORD);
            break;
        case GotoType::Break:
            printToken("break;", KEYWORD);
            break;
        case GotoType::Continue:
            printToken("continue;", KEYWORD);
            break;
        }
    }
}

void StructTreePrinter::printStructBlockSequence(StructBlockSequence* blockSeq) {
    for (auto block : blockSeq->getBlocks()) {
        printStructBlock(block);
        if (block != blockSeq->getBlocks().back())
            newLine();
    }
}

void StructTreePrinter::printStructBlockIf(StructBlockIf* block) {
    printStructBlock(block->getCondBlock());
    newLine();
    printToken("if (", KEYWORD);
    if (block->isInverted())
        printToken("!", KEYWORD);
    m_conditionPrinter(block->getCondBlock()->getPcodeBlock());
    printToken(") {", KEYWORD);
    startBlock();
    newLine();
    printStructBlock(block->getThenBlock());
    endBlock();
    newLine();
    printToken("}", KEYWORD);
    if (block->getElseBlock()) {
        printToken(" else {", KEYWORD);
        startBlock();
        newLine();
        printStructBlock(block->getElseBlock());
        endBlock();
        newLine();
        printToken("}", KEYWORD);
    }
}

void StructTreePrinter::printStructBlockWhile(StructBlockWhile* block) {
    printToken("while (", KEYWORD);
    printToken("true", KEYWORD);
    printToken(") {", KEYWORD);
    startBlock();
    newLine();
    printStructBlock(block->getBodyBlock());
    endBlock();
    newLine();
    printToken("}", KEYWORD);
}

StructTreePrinter::PrinterFunction StructTreePrinter::CodePrinter(pcode::Printer* pcodePrinter) {
    return std::function([pcodePrinter](pcode::Block* block) {
        pcodePrinter->printToken("// Block ", pcode::Printer::COMMENT);
        pcodePrinter->printToken(block->getName(), pcode::Printer::COMMENT);

        auto instructions = block->getInstructions();
        if (block->getLastInstruction()->isBranching()) {
            // remove any jump
            instructions.erase(std::prev(instructions.end()));
        }
        if (!instructions.empty())
            pcodePrinter->newLine();
        for (const auto& [offset, instruction] : instructions) {
            pcodePrinter->printInstruction(instruction);
            if (instruction != instructions.rbegin()->second)
                pcodePrinter->newLine();
        }
    });
}

StructTreePrinter::PrinterFunction StructTreePrinter::ConditionPrinter(pcode::Printer* pcodePrinter) {
    return std::function([pcodePrinter](pcode::Block* block) {
        auto instr = block->getLastInstruction();
        assert(instr->isBranching());
        pcodePrinter->printVarnode(instr->getInput1());
    });
}
