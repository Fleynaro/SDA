#include "DecGraphCondBlockOptimization.h"

using namespace CE::Decompiler;

CE::Decompiler::Optimization::GraphCondBlockOptimization::GraphCondBlockOptimization(DecompiledCodeGraph* decGraph)
	: GraphModification(decGraph)
{}

void CE::Decompiler::Optimization::GraphCondBlockOptimization::start() {
	doBlockJoining();

	//recalculate levels because some blocks can be removed
	m_decGraph->recalculateLevelsForBlocks();
}

void CE::Decompiler::Optimization::GraphCondBlockOptimization::doBlockJoining() {
	//join conditions and remove useless blocks
	for (auto it = m_decGraph->getDecompiledBlocks().rbegin(); it != m_decGraph->getDecompiledBlocks().rend(); it++) {
		auto block = *it;
		while (auto removedBlock = joinCondition(block)) {
			optimizeConditionDecBlock(block);
			m_decGraph->removeDecompiledBlock(removedBlock);
			it = m_decGraph->getDecompiledBlocks().rbegin();
		}
	}
}

void CE::Decompiler::Optimization::GraphCondBlockOptimization::optimizeConditionDecBlock(DecBlock* block) {
	if (!block->isCondition())
		return;
	auto delta = block->getNextNearBlock()->m_level - block->m_level;
	if (delta >= 1 && delta < block->getNextFarBlock()->m_level - block->m_level)
		return;
	block->swapNextBlocks();
	block->getNoJumpCondition()->inverse();
}

DecBlock* CE::Decompiler::Optimization::GraphCondBlockOptimization::joinCondition(DecBlock* block) {
	if (!block->isCondition())
		return nullptr;

	auto removedBlock = block->getNextNearBlock();
	auto mutualBlock = block->getNextFarBlock();
	if (!removedBlock->hasNoCode() || !removedBlock->isCondition() || removedBlock->getRefBlocksCount() != 1)
		return nullptr;

	DecBlock* targetBlock = nullptr;
	auto removedBlockNoJmpCond = removedBlock->getNoJumpCondition();
	if (removedBlock->getNextNearBlock() == mutualBlock) {
		targetBlock = removedBlock->getNextFarBlock();
		removedBlockNoJmpCond->inverse();
	}
	else if (removedBlock->getNextFarBlock() == mutualBlock) {
		targetBlock = removedBlock->getNextNearBlock();
	}
	if (!targetBlock)
		return nullptr;

	block->setNoJumpCondition(new CompositeCondition(block->getNoJumpCondition(), removedBlockNoJmpCond, CompositeCondition::And));
	block->setNextNearBlock(targetBlock);
	removedBlock->removeRefBlock(block);
	removedBlock->setNoJumpCondition(nullptr);
	return removedBlock;
}
