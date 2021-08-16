#pragma once
#include "DecLinearViewBlocks.h"
#include <set>
#include <map>

namespace CE::Decompiler::LinearView
{
	class Converter
	{
	public:
		struct Loop {
			DecBlock* m_startBlock;
			DecBlock* m_endBlock;
			std::set<DecBlock*> m_blocks;

			Loop(DecBlock* startBlock, DecBlock* endBlock)
				: m_startBlock(startBlock), m_endBlock(endBlock)
			{}
		};

		struct Cycle {
			DecBlock* m_startBlock;
			DecBlock* m_endBlock;
			std::set<DecBlock*> m_blocks;

			Cycle(DecBlock* startBlock = nullptr, DecBlock* endBlock = nullptr)
				: m_startBlock(startBlock), m_endBlock(endBlock)
			{}
		};

		struct VisitedBlockInfo {
			int m_enterCount = 0;
			std::list<DecBlock*> m_passedBlocks;
		};

		Converter(DecompiledCodeGraph* decCodeGraph)
			: m_decCodeGraph(decCodeGraph)
		{}

		void start() {
			// finding all loops
			const auto startBlock = m_decCodeGraph->getStartBlock();
			std::map<DecBlock*, VisitedBlockInfo> visitedBlocks;
			std::list<DecBlock*> passedBlocks;
			findAllLoops(startBlock, visitedBlocks, passedBlocks);

			// converting the decompiled graph into block lists
			m_blockList = new BlockList;
			std::set<DecBlock*> usedDecBlocks;
			std::set<DecBlock*> createdCycleDecBlocks;
			convert(m_blockList, startBlock, usedDecBlocks, createdCycleDecBlocks);

			// process all goto
			for (const auto& [blockList, nextBlock] : m_goto) {
				//if a condition block is above then not set goto as it is excess
				auto blocks = blockList->getBlocks();
				if (!blocks.empty())
					if (dynamic_cast<ConditionBlock*>(*blocks.rbegin()))
						continue;
				// set goto
				const auto block = m_blockList->findBlock(nextBlock);
				if (block != nullptr) {
					blockList->setGoto(block);
				}
			}
		}

		BlockList* getBlockList() const
		{
			return m_blockList;
		}
	private:
		DecompiledCodeGraph* m_decCodeGraph;
		std::map<DecBlock*, Loop> m_loops;
		std::map<DecBlock*, Cycle> m_cycles;
		std::list<std::pair<BlockList*, DecBlock*>> m_goto;
		BlockList* m_blockList;

		void convert(BlockList* const blockList, DecBlock* const decBlock, std::set<DecBlock*>& usedDecBlocks, std::set<DecBlock*>& createdCycleDecBlocks) {
			// need for filling it with new dec. blocks they have to be visited in the next recursive call
			std::list<std::pair<BlockList*, DecBlock*>> nextDecBlocksToFillBlockLists;
			
			auto curDecBlock = decBlock;
			while (curDecBlock != nullptr) {
				if (usedDecBlocks.count(curDecBlock) != 0) {
					// if go to the already visited block, don't process it again, just refer to it
					m_goto.emplace_back(blockList, curDecBlock);
					break;
				}
				DecBlock* nextBlock = nullptr;

				if (curDecBlock->isCycle() && createdCycleDecBlocks.count(curDecBlock) == 0) {
					createdCycleDecBlocks.insert(curDecBlock);
					const auto it = m_cycles.find(curDecBlock);
					if (it != m_cycles.end()) {
						const auto& cycle = it->second;
						const auto startCycleBlock = cycle.m_startBlock;
						const auto endCycleBlock = cycle.m_endBlock;

						// let the cycle be a {do-while} by default
						bool isDoWhileCycleBetter = true;
						// {while} cycle has always the first block that's conditional one($1), to get out of the cycle's scope($2)
						if (startCycleBlock->isCondition() && startCycleBlock->hasNoCode()) { // check $1
							if (cycle.m_blocks.count(startCycleBlock->getNextFarBlock()) == 0) { // check $2
								isDoWhileCycleBetter = false;

								// need to make the cycle body bigger (e.g. return statement can be inside the cycle, not after it)
								if (endCycleBlock->isCondition()) {
									if (startCycleBlock->getNextFarBlock()->m_maxHeight < endCycleBlock->getNextNearBlock()->m_maxHeight) {
										isDoWhileCycleBetter = true;
									}
								}
							}
						}

						if (!isDoWhileCycleBetter) {
							const auto whileCycleBlock = new WhileCycleBlock(decBlock->getJumpTopNode(), false);
							blockList->addBlock(new CodeBlock(curDecBlock));
							blockList->addBlock(whileCycleBlock);
							nextDecBlocksToFillBlockLists.emplace_front(whileCycleBlock->m_mainBranch, startCycleBlock->getNextNearBlock());
							nextBlock = startCycleBlock->getNextFarBlock();
						}
						else {
							WhileCycleBlock* whileCycleBlock;
							// check if a conditional block to exit the cycle is in the end or in the middle
							if (endCycleBlock->isCondition()) {
								whileCycleBlock = new WhileCycleBlock(decBlock->getJumpTopNode(), true);
								nextBlock = endCycleBlock->getNextNearBlock();
							}
							else {
								// the infinite cycle with a condition in the middle to exit it
								whileCycleBlock = new WhileCycleBlock(nullptr, true);
								// finding a conditional block($1) that refers to a non-cycle block($2)
								for (auto cycleBlock : cycle.m_blocks) {
									const auto farBlock = cycleBlock->getNextFarBlock();
									if (farBlock && cycle.m_blocks.count(farBlock) == 0) { // check $1 and $2
										nextBlock = farBlock;
										if (farBlock->getRefBlocksCount() != 1) {
											break;
										}
									}
								}
							}
							blockList->addBlock(whileCycleBlock);
							nextDecBlocksToFillBlockLists.emplace_front(whileCycleBlock->m_mainBranch, startCycleBlock);
							curDecBlock = nullptr;
						}
					}
				}
				else if (curDecBlock->isCondition()) {
					// as curDecBlock is conditional, find corresponding loop and set loop.endBlock as nextBlock
					const auto it = m_loops.find(curDecBlock);
					if (it != m_loops.end()) {
						const auto& loop = it->second;
						nextBlock = loop.m_endBlock;
						for (auto block : loop.m_blocks) {
							if (block == loop.m_endBlock)
								continue;
							// if one of the blocks in the loop have already been used
							if (usedDecBlocks.count(block) != 0) {
								nextBlock = nullptr;
								break;
							}
						}
					}

					const auto condBlock = new ConditionBlock(curDecBlock->getJumpTopNode());
					blockList->addBlock(new CodeBlock(curDecBlock));
					blockList->addBlock(condBlock);
					if (nextBlock) {
						// common {if-else} block
						nextDecBlocksToFillBlockLists.emplace_back(condBlock->m_mainBranch, curDecBlock->getNextNearBlock());
						nextDecBlocksToFillBlockLists.emplace_back(condBlock->m_elseBranch, curDecBlock->getNextFarBlock());
					}
					else {
						// need make the block list more linearly (for different if-checks)
						/*
						 * Before:
						 * if() {
						 *	block 1
						 * } else {
						 *	block 2
						 * }
						 * 
						 * After:
						 * if() {
						 *  block 1
						 *	return;
						 * }
						 * block 2
						 */
						auto blockInCond = curDecBlock->getNextNearBlock();
						auto blockBelowCond = curDecBlock->getNextFarBlock();
						if (blockInCond->m_maxHeight > blockBelowCond->m_maxHeight || usedDecBlocks.count(blockInCond) != 0) {
							std::swap(blockInCond, blockBelowCond);
							condBlock->m_cond->inverse();
						}
						nextDecBlocksToFillBlockLists.emplace_back(condBlock->m_mainBranch, blockInCond);
						nextDecBlocksToFillBlockLists.emplace_back(blockList, blockBelowCond);
						m_goto.emplace_back(condBlock->m_elseBranch, blockBelowCond);
					}
				}
				else {
					blockList->addBlock(new CodeBlock(curDecBlock));
					nextBlock = curDecBlock->getNextBlock();
				}

				// mark as used
				if (curDecBlock) {
					usedDecBlocks.insert(curDecBlock);
				}
				// go to next block
				curDecBlock = nextBlock;
			}

			// go to the next block lists for filling it with remaining dec. blocks
			for (const auto& [blockList, decBlock] : nextDecBlocksToFillBlockLists) {
				convert(blockList, decBlock, usedDecBlocks, createdCycleDecBlocks);
			}
		}

		void findAllLoops(DecBlock* block, std::map<DecBlock*, VisitedBlockInfo>& visitedBlocks, std::list<DecBlock*>& passedBlocks) {	
			bool goNext = true;
			const auto refHighBlocksCount = block->getRefHighBlocksCount();
			if (refHighBlocksCount >= 2) {
				if (visitedBlocks.find(block) == visitedBlocks.end()) {
					visitedBlocks.insert(std::make_pair(block, VisitedBlockInfo()));
				}
				auto& visitedBlock = visitedBlocks[block];
				
				visitedBlock.m_enterCount++;
				if (visitedBlock.m_enterCount < refHighBlocksCount) {
					goNext = false;
				}

				auto& blocks = visitedBlock.m_passedBlocks;
				blocks.insert(blocks.end(), passedBlocks.begin(), passedBlocks.end());

				if (visitedBlock.m_enterCount >= 2) {
					blocks.sort([](const DecBlock* block1, const DecBlock* block2) {
						return block1->m_level < block2->m_level && block1 != block2; //todo: here there are some issues
						});

					//detect a loop and remove duplicates
					auto startLoopBlockIt = blocks.end();
					for (auto it = std::next(blocks.begin()); it != blocks.end(); it++) {
						auto prevBlockIt = std::prev(it);
						if (*it == *prevBlockIt) {
							startLoopBlockIt = it;
							blocks.erase(prevBlockIt);
						}
					}

					//if a loop detected
					if (startLoopBlockIt != blocks.end()) {
						Loop loop(*startLoopBlockIt, block);
						for (auto it = startLoopBlockIt; it != blocks.end(); it++) {
							loop.m_blocks.insert(*it);
						}
						loop.m_blocks.insert(block);
						m_loops.insert(std::make_pair(*startLoopBlockIt, loop));
						//todo: blocks inside the loop but are not refering to the end of the loop are ignoring, the loop is not entire
					}

					if (goNext) {
						passedBlocks = blocks;
					}
				}
			}

			if (goNext) {
				passedBlocks.push_back(block);

				DecBlock* startCycleBlock = nullptr;
				for (auto nextBlock : block->getNextBlocks()) {
					if (nextBlock->m_level <= block->m_level) {
						startCycleBlock = nextBlock;
						continue;
					}
					findAllLoops(nextBlock, visitedBlocks, passedBlocks);
				}

				if (startCycleBlock)
				{
					if (m_cycles.find(startCycleBlock) == m_cycles.end()) {
						Cycle cycle(startCycleBlock, block);
						m_cycles.insert(std::make_pair(startCycleBlock, cycle));
					}
					auto& cycle = m_cycles[startCycleBlock];
					cycle.m_endBlock = std::max(cycle.m_endBlock, block);
					bool isBlockInCycle = false;
					for (auto passedBlock : passedBlocks) {
						if (passedBlock == cycle.m_startBlock)
							isBlockInCycle = true;
						if (isBlockInCycle) {
							cycle.m_blocks.insert(passedBlock);
						}
					}
				}

				for (auto it = passedBlocks.begin(); it != passedBlocks.end(); it++) {
					if (*it == block) {
						passedBlocks.erase(it, passedBlocks.end());
						break;
					}
				}
			}
		}
	};
};