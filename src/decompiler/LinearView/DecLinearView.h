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
			auto startBlock = m_decCodeGraph->getStartBlock();
			std::map<DecBlock*, VisitedBlockInfo> visitedBlocks;
			std::list<DecBlock*> passedBlocks;
			findAllLoops(startBlock, visitedBlocks, passedBlocks);

			m_blockList = new BlockList;
			std::set<DecBlock*> usedBlocks;
			std::set<DecBlock*> createdCycleBlocks;
			convert(m_blockList, startBlock, usedBlocks, createdCycleBlocks);

			for (auto it : m_goto) {
				auto blockList = it.first;
				auto nextBlock = it.second;
				//if a condition block is above then not set goto as it is excess
				if (!blockList->getBlocks().empty())
					if (dynamic_cast<Condition*>(*std::prev(blockList->getBlocks().end())))
						continue;

				auto block = m_blockList->findBlock(nextBlock);
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

		void convert(BlockList* blockList, DecBlock* decBlock, std::set<DecBlock*>& usedBlocks, std::set<DecBlock*>& createdCycleBlocks) {
			std::list<std::pair<BlockList*, DecBlock*>> nextBlocksToFill;
			
			auto curDecBlock = decBlock;
			while (curDecBlock != nullptr) {
				if (usedBlocks.count(curDecBlock) != 0) {
					m_goto.push_back(std::make_pair(blockList, curDecBlock));
					break;
				}
				DecBlock* nextBlock = nullptr;

				if (curDecBlock->isCycle() && createdCycleBlocks.count(curDecBlock) == 0) {
					createdCycleBlocks.insert(curDecBlock);
					auto it = m_cycles.find(curDecBlock);
					if (it != m_cycles.end()) {
						auto& cycle = it->second;
						auto startCycleBlock = cycle.m_startBlock;
						auto endCycleBlock = cycle.m_endBlock;

						bool isDoWhileCycleBetter = true;
						if (startCycleBlock->isCondition() && startCycleBlock->hasNoCode()) {
							auto nextBlockAfterCycle1 = startCycleBlock->getNextFarBlock();
							if (cycle.m_blocks.count(nextBlockAfterCycle1) == 0) {
								isDoWhileCycleBetter = false;

								if (endCycleBlock->isCondition()) {
									auto nextBlockAfterCycle2 = endCycleBlock->getNextNearBlock();
									if (nextBlockAfterCycle1->m_maxHeight < nextBlockAfterCycle2->m_maxHeight) {
										isDoWhileCycleBetter = true;
									}
								}
							}
						}

						if (!isDoWhileCycleBetter) {
							WhileCycle* whileCycle = new WhileCycle(startCycleBlock, false);
							blockList->addBlock(whileCycle);
							nextBlocksToFill.push_front(std::make_pair(whileCycle->m_mainBranch, startCycleBlock->getNextNearBlock()));
							nextBlock = startCycleBlock->getNextFarBlock();
						}
						else {
							WhileCycle* whileCycle;
							if (endCycleBlock->isCondition()) {
								whileCycle = new WhileCycle(endCycleBlock, true);
								nextBlock = endCycleBlock->getNextNearBlock();
							}
							else {
								whileCycle = new WhileCycle(endCycleBlock, true, true);
								for (auto cycleBlock : cycle.m_blocks) {
									auto farBlock = cycleBlock->getNextFarBlock();
									if (farBlock && cycle.m_blocks.count(farBlock) == 0) {
										nextBlock = farBlock;
										if (farBlock->getRefBlocksCount() != 1) {
											break;
										}
									}
								}
							}
							blockList->addBlock(whileCycle);
							nextBlocksToFill.push_front(std::make_pair(whileCycle->m_mainBranch, startCycleBlock));
							curDecBlock = endCycleBlock;
						}
					}
				}
				else if (curDecBlock->isCondition()) {
					auto it = m_loops.find(curDecBlock);
					if (it != m_loops.end()) {
						auto& loop = it->second;
						nextBlock = loop.m_endBlock;
						for (auto block : loop.m_blocks) {
							if (block == loop.m_endBlock)
								continue;
							if (usedBlocks.count(block) != 0) {
								nextBlock = nullptr;
								break;
							}
						}
					}

					auto cond = new Condition(curDecBlock);
					blockList->addBlock(cond);
					if (nextBlock) {
						nextBlocksToFill.push_back(std::make_pair(cond->m_mainBranch, curDecBlock->getNextNearBlock()));
						nextBlocksToFill.push_back(std::make_pair(cond->m_elseBranch, curDecBlock->getNextFarBlock()));
					}
					else {
						auto blockInCond = curDecBlock->getNextNearBlock();
						auto blockBelowCond = curDecBlock->getNextFarBlock();
						if (blockInCond->m_maxHeight > blockBelowCond->m_maxHeight || usedBlocks.count(blockInCond) != 0) {
							std::swap(blockInCond, blockBelowCond);
							cond->m_cond->inverse();
						}
						nextBlocksToFill.push_back(std::make_pair(cond->m_mainBranch, blockInCond));
						nextBlocksToFill.push_back(std::make_pair(blockList, blockBelowCond));
						m_goto.push_back(std::make_pair(cond->m_elseBranch, blockBelowCond));
					}
				}
				else {
					blockList->addBlock(new Block(curDecBlock));
					nextBlock = curDecBlock->getNextBlock();
				}

				if (curDecBlock) {
					usedBlocks.insert(curDecBlock);
				}
				curDecBlock = nextBlock;
			}

			for (auto block : nextBlocksToFill) {
				convert(block.first, block.second, usedBlocks, createdCycleBlocks);
			}
		}

		void findAllLoops(DecBlock* block, std::map<DecBlock*, VisitedBlockInfo>& visitedBlocks, std::list<DecBlock*>& passedBlocks) {	
			bool goNext = true;
			auto refHighBlocksCount = block->getRefHighBlocksCount();
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