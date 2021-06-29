#pragma once
#include "../SdaGraphModification.h"
#include "../../Graph/DecCodeGraphBlockFlowIterator.h"

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

			void clear() {
				for (auto& memValue : m_memValues) {
					delete memValue.m_topNode;
				}

				for (auto& pair : m_memVarSnapshots) {
					auto& memSnapshot = pair.second;
					delete memSnapshot.m_snapshotValue;
				}
			}

			SdaTopNode* getMemValue(const MemLocation& location) const {
				for (auto it = m_memValues.begin(); it != m_memValues.end(); it++) {
					if (it->m_location->equal(location)) { // todo: intersect better with <<, >> operation
						return it->m_topNode;
					}
				}
				return nullptr;
			}

			void addMemValue(const MemLocation& location, ISdaNode* sdaNode) {
				auto newLocation = createNewLocation(location);
				MemoryValue memoryValue;
				memoryValue.m_location = newLocation;
				memoryValue.m_topNode = new SdaTopNode(sdaNode);
				m_memValues.push_back(memoryValue);
			}

			MemLocation* createNewLocation(const MemLocation& location) {
				//clear all location that are intersecting this one
				auto it = m_memValues.begin();
				while(it != m_memValues.end()) {
					bool isIntersecting = it->m_location->intersect(location);
					if (!isIntersecting) {
						//stack_0x30 = stack_0x100 (location of stack_0x100 can be changed)
						if (auto locSnapshotValue = dynamic_cast<IMappedToMemory*>(it->m_topNode->getSdaNode())) {
							if (!locSnapshotValue->isAddrGetting()) {
								try {
									MemLocation snapshotValueLocation;
									locSnapshotValue->getLocation(snapshotValueLocation);
									if (snapshotValueLocation.intersect(location)) {
										isIntersecting = true;
									}
								}
								catch (std::exception&) {}
							}
						}
					}

					if (isIntersecting) {
						delete it->m_topNode;
						it = m_memValues.erase(it);
					}
					else {
						it++;
					}
				}
				
				//mark the input location as used within the current context
				m_usedMemLocations.push_back(location);
				return &(*m_usedMemLocations.rbegin()); //dangerous: important no to copy the mem ctx anywhere
			}

			int getLastUsedMemLocIdx() {
				return (int)m_usedMemLocations.size() - 1;
			}

			// check intersecting {location} with locations created from {firstUsedMemLocIdx} to {lastUsedMemLocIdx} state indexes
			bool hasUsed(const MemLocation& location, int firstUsedMemLocIdx = -1, int lastUsedMemLocIdx = -1) {
				for (const auto& loc : m_usedMemLocations) {
					if (lastUsedMemLocIdx-- == -1)
						break;
					if (firstUsedMemLocIdx >= 0) {
						firstUsedMemLocIdx--;
						continue;
					}
					if (loc.intersect(location)) {
						return true;
					}
				}
				return false;
			}
		};
		
		std::map<DecBlock*, MemoryContext> m_memoryContexts;
		std::list<SdaSymbolLeaf*> m_removedSymbolLeafs;
	public:
		SdaGraphMemoryOptimization(SdaCodeGraph* sdaCodeGraph)
			: SdaGraphModification(sdaCodeGraph)
		{}

		void start() override {
			initEveryMemCtxForEachBlocks();
			optimizeAllBlocksUsingMemCtxs();

			for (auto symbolLeaf : m_removedSymbolLeafs) {
				delete symbolLeaf;
			}

			for (auto& pair : m_memoryContexts) {
				auto& memCtx = pair.second;
				memCtx.clear();
			}
		}

	private:
		//just fill every memory context up for each block
		void initEveryMemCtxForEachBlocks() {
			for (auto block : m_sdaCodeGraph->getDecGraph()->getDecompiledBlocks()) {
				m_memoryContexts[block] = MemoryContext();
				MemoryContextInitializer memoryContextInitializer(block, &m_memoryContexts[block]);
				memoryContextInitializer.start();
			}
		}

		//optimize all blocks using filled up memory contexts on prev step
		void optimizeAllBlocksUsingMemCtxs() {
			for (auto block : m_sdaCodeGraph->getDecGraph()->getDecompiledBlocks()) {
				optimizeBlock(block, &m_memoryContexts[block]);
			}
		}

		void optimizeBlock(DecBlock* block, MemoryContext* memCtx) {
			for (auto& memVarInfo : memCtx->m_memVars) {
				ISdaNode* newNode = nullptr;
				auto memSnapshot = memVarInfo.m_memSnapshot;
				//if memSnapshot is in the current block
				if (memSnapshot) {
					newNode = getSnapshotValue(block, memCtx, memSnapshot, memVarInfo.m_lastUsedMemLocIdx);
					if (!newNode) {
						newNode = getLocatableNode(memCtx, memSnapshot, memVarInfo.m_lastUsedMemLocIdx);
					}
				}
				else {
					//find memSnapshot in blocks above
					auto pair = findBlockAndMemSnapshotByMemVar(memVarInfo.m_memVar);
					auto blockAbove = pair.first;
					auto memSnapshot = pair.second;
					if (blockAbove) {
						auto memCtx = &m_memoryContexts[blockAbove];
						auto lastUsedMemLocIdx = memCtx->getLastUsedMemLocIdx();
						auto newPossibleNode = getSnapshotValue(blockAbove, memCtx, memSnapshot, lastUsedMemLocIdx);
						if (auto locNewPossibleNode = dynamic_cast<ILocatable*>(newPossibleNode)) {
							try {
								MemLocation newPossibleNodeLocation;
								locNewPossibleNode->getLocation(newPossibleNodeLocation);
								if (canBlockBeReachedThroughLocation(block, blockAbove, &newPossibleNodeLocation)) {
									newNode = newPossibleNode;
								}
							}
							catch (std::exception&) {}
						}

						if (!newNode) {
							newPossibleNode = getLocatableNode(memCtx, memSnapshot, lastUsedMemLocIdx);
							if (canBlockBeReachedThroughLocation(block, blockAbove, &memSnapshot->m_location)) {
								newNode = newPossibleNode;
							}
						}
					}
				}

				if (newNode) {
					//replace the symbol with the concrete value (e.g. reading some memory location)
					auto newClonedNode = newNode->clone();
					memVarInfo.m_symbolLeaf->replaceWith(newClonedNode);
					m_removedSymbolLeafs.push_back(memVarInfo.m_symbolLeaf);
				}
			}
		}

		ISdaNode* getSnapshotValue(DecBlock* block, MemoryContext* memCtx, MemoryContext::MemSnapshot* memSnapshot, int lastUsedMemLocIdx) {
			if (memSnapshot->m_snapshotValue) {
				if (auto locSnapshotValue = dynamic_cast<ILocatable*>(memSnapshot->m_snapshotValue->getNode())) {
					try {
						/*
							entity1.vec.x = entity1.vec.z
							memVar1488 = entity1.vec.x (m_snapshotValue = entity1.vec.z)
							entity1.vec.z = 1 (changed!)
							return memVar1488; (m_snapshotValue != entity1.vec.z)
						*/
						MemLocation snapshotValueLocation;
						locSnapshotValue->getLocation(snapshotValueLocation);
						if (memCtx->hasUsed(snapshotValueLocation, memSnapshot->m_lastUsedMemLocIdx, lastUsedMemLocIdx)) {
							return nullptr;
						}
					}
					catch (std::exception&) {} // todo: dangerous, need to remove
				}
				return memSnapshot->m_snapshotValue->getSdaNode();
			}
			else {
				if (auto foundNode = findValueNodeInBlocksAbove(block, &memSnapshot->m_location)) {
					return foundNode;
				}
			}
			return nullptr;
		}

		ISdaNode* getLocatableNode(MemoryContext* memCtx, MemoryContext::MemSnapshot* memSnapshot, int lastUsedMemLocIdx) {
			if (memSnapshot->m_locatableNode) {
				if (!memCtx->hasUsed(memSnapshot->m_location, memSnapshot->m_lastUsedMemLocIdx, lastUsedMemLocIdx)) {
					return memSnapshot->m_locatableNode;
				}
			}
			return nullptr;
		}

		std::pair<DecBlock*, MemoryContext::MemSnapshot*> findBlockAndMemSnapshotByMemVar(Symbol::MemoryVariable* memVar) {
			for (auto block : m_sdaCodeGraph->getDecGraph()->getDecompiledBlocks()) {
				auto& memVarToMemLocation = m_memoryContexts[block].m_memVarSnapshots;
				auto it = memVarToMemLocation.find(memVar);
				if (it != memVarToMemLocation.end()) {
					return std::pair(block, &it->second);
				}
			}
			return std::pair(nullptr, nullptr);
		}

		//finalBlock can be reached if go to it over path(from startBlock) that doesn't contain changes of the memory location
		bool canBlockBeReachedThroughLocation(DecBlock* startBlock, DecBlock* finalBlock, MemLocation* memLoc) {
			BlockFlowIterator blockFlowIterator(startBlock);
			while (blockFlowIterator.hasNext()) {
				blockFlowIterator.m_considerLoop = false;
				if (blockFlowIterator.isStartBlock())
					continue;
				auto& blockInfo = blockFlowIterator.next();
				if (blockInfo.m_block == finalBlock) {
					return blockInfo.hasMaxPressure();
				}
				auto memCtx = &m_memoryContexts[blockInfo.m_block];
				if (memCtx->hasUsed(*memLoc))
					break;
			}

			return false;
		}

		ISdaNode* findValueNodeInBlocksAbove(DecBlock* startBlock, MemLocation* memLoc) {
			BlockFlowIterator blockFlowIterator(startBlock);
			while (blockFlowIterator.hasNext()) {
				blockFlowIterator.m_considerLoop = false;
				if (blockFlowIterator.isStartBlock())
					continue;
				auto& blockInfo = blockFlowIterator.next();
				auto memCtx = &m_memoryContexts[blockInfo.m_block];
				if (blockInfo.hasMaxPressure()) {
					if (auto valueTopNode = memCtx->getMemValue(*memLoc))
						return valueTopNode->getSdaNode();
				}
				if (memCtx->hasUsed(*memLoc))
					break;
			}

			return nullptr;
		}

		// for the specified block it fill the memory context by some values during simulation of execution
		class MemoryContextInitializer
		{
			DecBlock* m_block;
			MemoryContext* m_memCtx; // it has to be filled
		public:
			MemoryContextInitializer(DecBlock* block, MemoryContext* memCtx)
				: m_block(block), m_memCtx(memCtx)
			{}

			void start() {
				// iterate over all lines of the code as if executing it
				for (auto topNode : m_block->getAllTopNodes()) {
					auto node = topNode->getNode();
					INode::UpdateDebugInfo(node);
					passNode(node);
				}
			}

		private:
			void passNode(INode* node) {
				if (auto assignmentNode = dynamic_cast<AssignmentNode*>(node)) {
					if (dynamic_cast<SdaSymbolLeaf*>(assignmentNode->getDstNode())) {
						passNode(assignmentNode->getSrcNode());
						return;
					}
				}
				node->iterateChildNodes([&](INode* childNode) {
					passNode(childNode);
					});

				if (auto sdaGenNode = dynamic_cast<SdaGenericNode*>(node)) {
					if (auto assignmentNode = dynamic_cast<AssignmentNode*>(sdaGenNode->getNode())) {
						auto dstSdaNode = dynamic_cast<ISdaNode*>(assignmentNode->getDstNode());
						auto srcSdaNode = dynamic_cast<ISdaNode*>(assignmentNode->getSrcNode());

						if (dstSdaNode && srcSdaNode) {
							//when writing some stuff into a memory location (entity2.vec.x = memVar1488 + 0.5)
							if (auto dstSdaLocNode = dynamic_cast<ILocatable*>(dstSdaNode)) {
								try {
									MemLocation dstLocation;
									dstSdaLocNode->getLocation(dstLocation);
									m_memCtx->addMemValue(dstLocation, srcSdaNode);
								}
								catch (std::exception&) {}
							}
							else {
								if (auto sdaSymbolLeaf = dynamic_cast<SdaSymbolLeaf*>(assignmentNode->getDstNode())) {
									if (auto memVar = dynamic_cast<Symbol::MemoryVariable*>(sdaSymbolLeaf->getDecSymbol())) {
										//when reading from a memory location into the memory symbol (memVar1488 = entity1.vec.x)
										if (auto srcSdaLocNode = dynamic_cast<ILocatable*>(srcSdaNode)) {
											try {
												MemLocation srcLocation;
												srcSdaLocNode->getLocation(srcLocation);
												MemoryContext::MemSnapshot memSnapshot;
												memSnapshot.m_location = srcLocation;
												memSnapshot.m_locatableNode = srcSdaLocNode;
												memSnapshot.m_lastUsedMemLocIdx = m_memCtx->getLastUsedMemLocIdx();
												auto snapshotValueTopNode = m_memCtx->getMemValue(srcLocation);
												if (snapshotValueTopNode) {
													// create snapshot of a value stored on the memory location {srcLocation}
													memSnapshot.m_snapshotValue = new SdaTopNode(snapshotValueTopNode->getSdaNode());
												}
												m_memCtx->m_memVarSnapshots[memVar] = memSnapshot;
											}
											catch (std::exception&) {}
										}
									}
								}
							}
						}
					}
				}

				//replace the internal memVar with a node value stored on some location for this memVar (return memVar1488; -> return entity1.vec.x;)
				if (auto sdaSymbolLeaf = dynamic_cast<SdaSymbolLeaf*>(node)) {
					if (auto memVar = dynamic_cast<Symbol::MemoryVariable*>(sdaSymbolLeaf->getDecSymbol())) {
						MemoryContext::MemVarInfo memVarInfo;
						memVarInfo.m_symbolLeaf = sdaSymbolLeaf;
						memVarInfo.m_memVar = memVar;
						memVarInfo.m_lastUsedMemLocIdx = m_memCtx->getLastUsedMemLocIdx();

						//if the symbol not found within block then it means to be declared in the blocks above
						auto it = m_memCtx->m_memVarSnapshots.find(memVar);
						if (it != m_memCtx->m_memVarSnapshots.end()) {
							auto memSnapshot = &it->second;
							memVarInfo.m_memSnapshot = memSnapshot;
						}
						m_memCtx->m_memVars.push_back(memVarInfo);
					}
				}

				//if the function call appeared then clear nearly all location as we dont know what memory this function affected
				if (auto sdaFunctionNode = dynamic_cast<SdaFunctionNode*>(node)) {
					MemLocation memAllLoc;
					memAllLoc.m_type = MemLocation::ALL; // clear all
					m_memCtx->createNewLocation(memAllLoc);
				}
			}
		};
	};
};