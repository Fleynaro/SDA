#pragma once
#include "DecLinearViewBlocks.h"

namespace CE::Decompiler::LinearView
{
	static void CalculateLinearLevelForBlockList(BlockList* blockList, int& level) {
		blockList->m_minLinearLevel = level;
		for (auto block : blockList->getBlocks()) {
			block->m_linearLevel = level++;

			// go to the next block lists
			if (auto blockListAgregator = dynamic_cast<IBlockListAgregator*>(block)) {	
				for (const auto blockList : blockListAgregator->getBlockLists()) {
					CalculateLinearLevelForBlockList(blockList, level);
				}
			}
		}
		blockList->m_maxLinearLevel = level;
	}

	static void CalculateBackOrderIdsForBlockList(BlockList* blockList, int orderId = 1) {
		//goto is a line (like not empty block)
		if (blockList->hasGoto())
			orderId++;

		const auto blocks = blockList->getBlocks();
		for (auto it = blocks.rbegin(); it != blocks.rend(); ++it) {
			const auto block = *it;
			orderId++;
			if (const auto whileBlock = dynamic_cast<WhileCycle*>(block)) {
				block->m_backOrderId = whileBlock->m_mainBranch->m_backOrderId = orderId;
				CalculateBackOrderIdsForBlockList(whileBlock->m_mainBranch, orderId);
				if (!whileBlock->m_mainBranch->getBlocks().empty()) {
					orderId = (*whileBlock->m_mainBranch->getBlocks().begin())->m_backOrderId;
				}
			}
			else {
				if (const auto condBlock = dynamic_cast<Condition*>(block)) {
					auto maxOrderId = orderId;
					for (const auto nextBlockList : condBlock->getBlockLists()) {
						nextBlockList->m_backOrderId = orderId - 1;
						CalculateBackOrderIdsForBlockList(nextBlockList, orderId - 1);
						if (!nextBlockList->getBlocks().empty()) {
							maxOrderId = std::max(maxOrderId, (*nextBlockList->getBlocks().begin())->getBackOrderId());
						}
					}
					orderId = maxOrderId;
				}
				block->m_backOrderId = orderId;
			}
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
			if (blockList->getMinLinearLevel() > farBlock->getLinearLevel() && !farBlock->m_decBlock->isCycle()) {
				// block swapping
				farBlockList->removeBlock(farBlock);
				blockList->addBlock(farBlock);
				blockList->m_goto = farBlockList->m_goto;
				farBlockList->m_goto = farBlock;
			}
		}

		// go to the next blocks
		for (const auto block : blockList->getBlocks()) {
			if (const auto blockListAgregator = dynamic_cast<IBlockListAgregator*>(block)) {
				for (const auto blockList : blockListAgregator->getBlockLists()) {
					OptimizeBlockOrderBlockList(blockList);
				}
			}
		}
	}

	static void OptimizeBlockList(BlockList* blockList) {
		int level = 1;
		CalculateLinearLevelForBlockList(blockList, level);
		OptimizeBlockOrderBlockList(blockList);
		level = 1;
		CalculateLinearLevelForBlockList(blockList, level);
		CalculateBackOrderIdsForBlockList(blockList);
	}
};