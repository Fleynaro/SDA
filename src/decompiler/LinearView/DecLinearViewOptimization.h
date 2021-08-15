#pragma once
#include "DecLinearViewBlocks.h"

namespace CE::Decompiler::LinearView
{
	static void CalculateLinearLevelForBlockList(BlockList* blockList, int& level) {
		blockList->m_minLinearLevel = level;
		for (auto block : blockList->getBlocks()) {
			block->m_linearLevel = level++;

			// go to the next block lists
			for (const auto blockList : block->getBlockLists()) {
				CalculateLinearLevelForBlockList(blockList, level);
			}
		}
		blockList->m_maxLinearLevel = level;
	}

	static void CalculateBackOrderIdsForBlockList(BlockList* blockList, int orderId = 1) {
		//goto is a line (like not empty block)
		if (blockList->hasGoto() && false) {
			/*
			 * while() {
			 *	if() {
			 *	 if() {
			 *	  goto label;
			 *	 }
			 *	 break;
			 *	}
			 *	i++; // label
			 * }
			 */
			orderId++;
		}

		const auto blocks = blockList->getBlocks();
		for (auto it = blocks.rbegin(); it != blocks.rend(); ++it) {
			const auto block = *it;
			if (const auto codeBlock = dynamic_cast<CodeBlock*>(block)) {
				if (!codeBlock->m_decBlock->hasNoCode()) {
					orderId++;
				}
			}
			else {
				auto maxOrderId = orderId;
				for (const auto nextBlockList : block->getBlockLists()) {
					nextBlockList->m_backOrderId = orderId;
					CalculateBackOrderIdsForBlockList(nextBlockList, orderId);
					if (const auto firstBlock = nextBlockList->getFirstBlock()) {
						maxOrderId = std::max(maxOrderId, firstBlock->m_backOrderId);
					}
				}
				orderId = maxOrderId + 1;
				// exception for do-while cycle
				if(const auto whileBlock = dynamic_cast<WhileCycleBlock*>(block)) {
					if (whileBlock->m_isDoWhileCycle) {
						orderId = maxOrderId;
					}
				}
			}
			block->m_backOrderId = orderId;
		}
	}

	// remove all goto operators that refer to upper blocks and swap blocks (not to be confused with a cycle)
	static void OptimizeBlockOrderBlockList(BlockList* blockList) {
		/*
		 * Before:
		 * if() {
		 *	block 1
		 * }
		 * if() {
		 *	block 2
		 *	goto {block 1}
		 * }
		 *
		 * After:
		 * if() {
		 *	goto {block 2}
		 * }
		 * if() {
		 *	block 2
		 * }
		 */
		
		if (const auto farBlock = blockList->m_goto) {
			const auto farBlockList = farBlock->m_blockList;
			if (blockList->m_minLinearLevel > farBlock->m_linearLevel && !farBlock->m_decBlock->isCycle()) {
				// block swapping
				farBlockList->removeBlock(farBlock);
				blockList->addBlock(farBlock);
				blockList->m_goto = farBlockList->m_goto;
				farBlockList->m_goto = farBlock;
			}
		}

		// go to the next blocks
		for (const auto block : blockList->getBlocks()) {
			for (const auto blockList : block->getBlockLists()) {
				OptimizeBlockOrderBlockList(blockList);
			}
		}
	}

	static void OptimizeBlockList(BlockList* blockList) {
		int level = 1;
		/*CalculateLinearLevelForBlockList(blockList, level);
		OptimizeBlockOrderBlockList(blockList);
		level = 1;*/
		CalculateLinearLevelForBlockList(blockList, level);
		CalculateBackOrderIdsForBlockList(blockList);
	}
};