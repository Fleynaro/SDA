#pragma once
#include "DecCodeGraphBlock.h"

namespace CE::Decompiler
{
	class BlockFlowIterator
	{
		const static uint64_t MaxPressure = 0x1000000000000000;
	public:
		struct BlockInfo {
			DecBlock* m_block;
			uint64_t m_pressure = 0x0;
			BitMask64 m_notNeedToReadMask; //it may happens that dont need reading from the block because its child blocks have been already read

			BlockInfo() = default;
			BlockInfo(DecBlock* block, uint64_t pressure, BitMask64 notNeedToReadMask)
				: m_block(block), m_pressure(pressure), m_notNeedToReadMask(notNeedToReadMask)
			{}

			bool hasMaxPressure() const
			{
				return m_pressure == MaxPressure;
			}
		};

	private:
		std::map<DecBlock*, BlockInfo> m_blockInfos; // blocks that have pressure and wait for sharing it with others
		std::list<BlockInfo> m_blocksOnOneLevel; // the batch of blocks of ONE level that have to be iterated over
		int m_iterCount = 0;

	public:
		bool m_considerLoop = true;
		bool m_distributePressure = true;

		BlockFlowIterator(DecBlock* startBlock, BitMask64 notNeedToReadMask = BitMask64(0));

		bool isStartBlock() const;

		bool hasNext();

		BlockInfo& next();

		void passThisBlockAgain();

	private:
		void addBlockInfo(DecBlock* block, uint64_t pressure, BitMask64 notNeedToReadMask);

		// add the bottommost blocks to the batch
		void defineBlocksOnOneLevel();

		// get level of the bottommost block
		int getBiggestLevel();

		// share its pressure for others and remove itself
		void distributePressure(BlockInfo blockInfo, bool considerLoop);
	};
};