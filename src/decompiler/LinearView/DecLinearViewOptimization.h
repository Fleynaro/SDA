#pragma once
#include "DecLinearViewBlocks.h"

namespace CE::Decompiler::LinearView
{
	static void CalculateLinearLevelForBlockList(BlockList* blockList, int& level) {
		blockList->m_minLinearLevel = level;
		for (auto block : blockList->getBlocks()) {
			block->m_linearLevel = level++;
			if (auto blockListAgregator = dynamic_cast<IBlockListAgregator*>(block)) {
				if (blockListAgregator->isInversed()) {
					level--;
				}
				for (auto blockList : blockListAgregator->getBlockLists()) {
					CalculateLinearLevelForBlockList(blockList, level);
				}
				if (blockListAgregator->isInversed()) {
					block->m_linearLevel = level++;
				}
			}
		}
		blockList->m_maxLinearLevel = level;
	}

	static void CalculateBackOrderIdsForBlockList(BlockList* blockList, int orderId = 1) {
		//goto is a line (like not empty block)
		if (blockList->hasGoto())
			orderId++;

		for (auto it = blockList->getBlocks().rbegin(); it != blockList->getBlocks().rend(); it++) {
			const auto block = *it;
			orderId++;
			if (auto blockListAgregator = dynamic_cast<IBlockListAgregator*>(block)) {
				if (blockListAgregator->isInversed()) {
					auto blockList = *blockListAgregator->getBlockLists().begin();
					block->m_backOrderId = blockList->m_backOrderId = orderId;
					CalculateBackOrderIdsForBlockList(blockList, orderId);
					if (!blockList->getBlocks().empty()) {
						orderId = (*blockList->getBlocks().begin())->m_backOrderId;
					}
				}
				else {
					auto maxOrderId = orderId;
					for (auto blockList : blockListAgregator->getBlockLists()) {
						blockList->m_backOrderId = orderId - 1;
						CalculateBackOrderIdsForBlockList(blockList, orderId - 1);
						if (!blockList->getBlocks().empty()) {
							maxOrderId = std::max(maxOrderId, (*blockList->getBlocks().begin())->getBackOrderId());
						}
					}
					orderId = maxOrderId;
					block->m_backOrderId = orderId;
				}
			}
			else {
				block->m_backOrderId = orderId;
			}
		}
	}

	static void OptimizeBlockOrderBlockList(BlockList* blockList) {
		auto farBlock = blockList->m_goto;
		if (farBlock) {
			auto farBlockList = farBlock->m_blockList;
			if (blockList->getMinLinearLevel() > farBlock->getLinearLevel() && !farBlock->m_decBlock->isCycle()) {
				farBlockList->removeBlock(farBlock);
				blockList->addBlock(farBlock);
				blockList->m_goto = farBlockList->m_goto;
				farBlockList->m_goto = farBlock;
			}
		}

		for (auto block : blockList->getBlocks()) {
			if (auto blockListAgregator = dynamic_cast<IBlockListAgregator*>(block)) {
				for (auto blockList : blockListAgregator->getBlockLists()) {
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