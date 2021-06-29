#pragma once
#include "DecGraphModification.h"

namespace CE::Decompiler::Optimization
{
	//Transforming sequance of simple blocks to single complex block with complex condition(&&, ||) for jump
	class GraphCondBlockOptimization : public GraphModification
	{
	public:
		GraphCondBlockOptimization(DecompiledCodeGraph* decGraph)
			: GraphModification(decGraph)
		{}

		void start() override {
			doBlockJoining();

			//recalculate levels because some blocks can be removed
			m_decGraph->recalculateLevelsForBlocks();
		}
	private:
		void doBlockJoining() {
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

		void optimizeConditionDecBlock(DecBlock* block) {
			if (!block->isCondition())
				return;
			auto delta = block->getNextNearBlock()->m_level - block->m_level;
			if (delta >= 1 && delta < block->getNextFarBlock()->m_level - block->m_level)
				return;
			block->swapNextBlocks();
			block->getNoJumpCondition()->inverse();
		}

		DecBlock* joinCondition(DecBlock* block) {
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
	};
};