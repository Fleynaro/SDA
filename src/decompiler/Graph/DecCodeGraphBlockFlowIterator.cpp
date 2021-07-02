#include "DecCodeGraphBlockFlowIterator.h"

using namespace CE::Decompiler;

// share its pressure for others and remove itself

CE::Decompiler::BlockFlowIterator::BlockFlowIterator(DecBlock* startBlock, BitMask64 notNeedToReadMask)
{
	addBlockInfo(startBlock, MaxPressure, notNeedToReadMask); //set the start block (with max pressure that have to be distributed)
}

bool CE::Decompiler::BlockFlowIterator::isStartBlock() {
	return m_iterCount == 1;
}

bool CE::Decompiler::BlockFlowIterator::hasNext() {
	//remove the first block from the current batch because it already grabbed by next() method
	if (!m_blocksOnOneLevel.empty()) {
		if (m_distributePressure) {
			distributePressure(*m_blocksOnOneLevel.begin(), m_considerLoop);
		}
		m_blocksOnOneLevel.pop_front();
	}

	//if the batch is empty then fill it up with new bottommost blocks of one level
	if (m_blocksOnOneLevel.empty()) {
		defineBlocksOnOneLevel();
	}

	//restore the default values
	m_considerLoop = true;
	m_distributePressure = true;
	m_iterCount++;
	return !m_blocksOnOneLevel.empty();
}

BlockFlowIterator::BlockInfo& CE::Decompiler::BlockFlowIterator::next() {
	return *m_blocksOnOneLevel.begin();
}

void CE::Decompiler::BlockFlowIterator::passThisBlockAgain() {
	m_blocksOnOneLevel.push_back(next());
	m_distributePressure = false;
}

void CE::Decompiler::BlockFlowIterator::addBlockInfo(DecBlock* block, uint64_t pressure, BitMask64 notNeedToReadMask) {
	m_blockInfos[block] = BlockInfo(block, pressure, notNeedToReadMask);
}

// add the bottommost blocks to the batch

void CE::Decompiler::BlockFlowIterator::defineBlocksOnOneLevel() {
	auto biggestLevel = getBiggestLevel();
	for (auto it : m_blockInfos) {
		auto block = it.first;
		auto& blockInfo = it.second;
		if (block->m_level == biggestLevel) { //find blocks with the highest level down
			m_blocksOnOneLevel.push_back(blockInfo);
		}
	}
}

// get level of the bottommost block

int CE::Decompiler::BlockFlowIterator::getBiggestLevel() {
	int biggestLevel = 0;
	for (auto it : m_blockInfos) {
		auto block = it.first;
		if (block->m_level > biggestLevel) {
			biggestLevel = it.first->m_level;
		}
	}
	return biggestLevel;
}

void CE::Decompiler::BlockFlowIterator::distributePressure(BlockInfo blockInfo, bool considerLoop) {
	auto block = blockInfo.m_block;
	m_blockInfos.erase(block); // block without pressure have to be removed (this pressure distributed for others)

							   //if the start block is cycle then distribute the pressure for all referenced blocks. Next time don't it.
	auto parentsCount = considerLoop ? block->getRefBlocksCount() : block->getRefHighBlocksCount();
	if (parentsCount > 0) {
		//calculate pressure for next blocks
		auto bits = (int)ceil(log2((double)parentsCount));
		auto addPressure = blockInfo.m_pressure >> bits;
		auto restAddPressure = addPressure * ((1 << bits) % parentsCount);

		//distribute the calculated pressure for each parent block
		for (auto parentBlock : block->getBlocksReferencedTo()) {
			if (!considerLoop && parentBlock->m_level >= block->m_level) //parent blocks must be higher than the current block
				continue;

			if (m_blockInfos.find(parentBlock) == m_blockInfos.end()) { // if this parent block firstly meets
				addBlockInfo(parentBlock, 0x0, blockInfo.m_notNeedToReadMask);
			}
			auto& parentBlockInfo = m_blockInfos[parentBlock];
			parentBlockInfo.m_pressure += addPressure + restAddPressure;
			parentBlockInfo.m_notNeedToReadMask = parentBlockInfo.m_notNeedToReadMask & blockInfo.m_notNeedToReadMask;
			restAddPressure = 0;
		}
	}
}
