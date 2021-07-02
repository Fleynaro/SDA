#pragma once
#include "../SdaGraphModification.h"
#include <decompiler/Graph/DecCodeGraphBlockFlowIterator.h>

namespace CE::Decompiler::Optimization
{
	using namespace ExprTree;

	class SdaGraphMemoryOptimization : public SdaGraphModification
	{
		class MemoryContext
		{
			struct MemoryValue {
				MemLocation* m_location;
				SdaTopNode* m_topNode;
			};
			std::list<MemoryValue> m_memValues;
		public:
			struct MemSnapshot {
				MemLocation m_location;
				ILocatable* m_locatableNode = nullptr;
				SdaTopNode* m_snapshotValue = nullptr;
				int m_lastUsedMemLocIdx; // saved memory state index
			};

			struct MemVarInfo {
				SdaSymbolLeaf* m_symbolLeaf;
				Symbol::MemoryVariable* m_memVar;
				MemSnapshot* m_memSnapshot = nullptr;
				int m_lastUsedMemLocIdx; // saved memory state index
			};

			//the result of memory copy working
			std::list<MemVarInfo> m_memVars;
			//locations in memory that are affected within the block(this ctx reffers to) in one or another way
			std::list<MemLocation> m_usedMemLocations;
			std::map<Symbol::MemoryVariable*, MemSnapshot> m_memVarSnapshots;

			void clear();

			SdaTopNode* getMemValue(const MemLocation& location) const;

			void addMemValue(const MemLocation& location, ISdaNode* sdaNode);

			MemLocation* createNewLocation(const MemLocation& location);

			int getLastUsedMemLocIdx();

			// check intersecting {location} with locations created from {firstUsedMemLocIdx} to {lastUsedMemLocIdx} state indexes
			bool hasUsed(const MemLocation& location, int firstUsedMemLocIdx = -1, int lastUsedMemLocIdx = -1);
		};
		
		std::map<DecBlock*, MemoryContext> m_memoryContexts;
		std::list<SdaSymbolLeaf*> m_removedSymbolLeafs;
	public:
		SdaGraphMemoryOptimization(SdaCodeGraph* sdaCodeGraph);

		void start() override;

	private:
		//just fill every memory context up for each block
		void initEveryMemCtxForEachBlocks();

		//optimize all blocks using filled up memory contexts on prev step
		void optimizeAllBlocksUsingMemCtxs();

		void optimizeBlock(DecBlock* block, MemoryContext* memCtx);

		ISdaNode* getSnapshotValue(DecBlock* block, MemoryContext* memCtx, MemoryContext::MemSnapshot* memSnapshot, int lastUsedMemLocIdx);

		ISdaNode* getLocatableNode(MemoryContext* memCtx, MemoryContext::MemSnapshot* memSnapshot, int lastUsedMemLocIdx);

		std::pair<DecBlock*, MemoryContext::MemSnapshot*> findBlockAndMemSnapshotByMemVar(Symbol::MemoryVariable* memVar);

		//finalBlock can be reached if go to it over path(from startBlock) that doesn't contain changes of the memory location
		bool canBlockBeReachedThroughLocation(DecBlock* startBlock, DecBlock* finalBlock, MemLocation* memLoc);

		ISdaNode* findValueNodeInBlocksAbove(DecBlock* startBlock, MemLocation* memLoc);

		// for the specified block it fill the memory context by some values during simulation of execution
		class MemoryContextInitializer
		{
			DecBlock* m_block;
			MemoryContext* m_memCtx; // it has to be filled
		public:
			MemoryContextInitializer(DecBlock* block, MemoryContext* memCtx);

			void start();

		private:
			void passNode(INode* node);
		};
	};
};