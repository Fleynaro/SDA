#pragma once
#include "DecCodeGraphBlock.h"
#include "DecPCodeGraph.h"

namespace CE::Decompiler
{
	class DecompiledCodeGraph
	{
	public:
		DecompiledCodeGraph(FunctionPCodeGraph* funcGraph)
			: m_funcGraph(funcGraph)
		{}

		~DecompiledCodeGraph() {
			for (auto block : m_decompiledBlocks) {
				delete block;
			}

			for (auto block : m_removedDecompiledBlocks) {
				delete block;
			}

			for (auto symbol : getSymbols()) {
				delete symbol;
			}
		}

		FunctionPCodeGraph* getFuncGraph() {
			return m_funcGraph;
		}

		DecBlock* getStartBlock() {
			return *getDecompiledBlocks().begin();
		}

		std::list<DecBlock*>& getDecompiledBlocks() {
			return m_decompiledBlocks;
		}

		std::list<Symbol::Symbol*>& getSymbols() {
			return m_symbols;
		}

		void removeDecompiledBlock(DecBlock* decBlock) {
			m_decompiledBlocks.remove(decBlock);
			m_removedDecompiledBlocks.push_back(decBlock);
			decBlock->disconnect();
		}

		void addSymbol(Symbol::Symbol* symbol) {
			m_symbols.push_back(symbol);
		}

		void removeSymbol(Symbol::Symbol* symbol) {
			m_symbols.remove(symbol);
		}

		void cloneAllExpr() {
			for (auto block : m_decompiledBlocks) {
				block->cloneAllExpr();
			}
		}

		void sortBlocksByLevel() {
			m_decompiledBlocks.sort([](DecBlock* a, DecBlock* b) {
				return a->m_level < b->m_level;
				});
		}

		void checkOnSingleParents() {
			for (const auto decBlock : getDecompiledBlocks()) {
				for (auto topNode : decBlock->getAllTopNodes()) {
					ExprTree::INode::UpdateDebugInfo(topNode->getNode());
					topNode->getNode()->checkOnSingleParents();
				}
			}
		}

		HS getHash() {
			HS hs;
			for (const auto decBlock : getDecompiledBlocks()) {
				for (auto topNode : decBlock->getAllTopNodes()) {
					hs = hs << topNode->getNode()->getHash();
				}
			}
			return hs;
		}

		// recalculate levels because some blocks can be removed (while parsing AND/OR block constructions)
		void recalculateLevelsForBlocks() {
			for (const auto decBlock : getDecompiledBlocks()) {
				decBlock->m_level = 0;
			}
			std::list<DecBlock*> path;
			DecompiledCodeGraph::CalculateLevelsForDecBlocks(getStartBlock(), path);
		}

		// calculate count of lines(height) for each block beginining from lower blocks (need as some score for linearization)
		static int CalculateHeightForDecBlocks(DecBlock* block) {
			int height = 0;
			for (auto nextBlock : block->getNextBlocks()) {
				if (nextBlock->m_level > block->m_level) { // to avoid loops
					auto h = CalculateHeightForDecBlocks(nextBlock);
					height = max(height, h);
				}
			}
			block->m_maxHeight = height + (int)block->getSeqAssignmentLines().size();
			return block->m_maxHeight;
		}
	private:
		FunctionPCodeGraph* m_funcGraph;
		std::list<DecBlock*> m_decompiledBlocks;
		std::list<DecBlock*> m_removedDecompiledBlocks;
		std::list<Symbol::Symbol*> m_symbols;

		// pass decompiled graph and calculate max distance from the root to each node (dec block). Similarly to asm graph!
		static void CalculateLevelsForDecBlocks(DecBlock* block, std::list<DecBlock*>& path) {
			if (block == nullptr)
				return;

			//check if there's a loop
			for (auto it = path.rbegin(); it != path.rend(); it++) {
				if (block == *it) {
					return;
				}
			}

			path.push_back(block);
			block->m_level = max(block->m_level, (int)path.size());
			CalculateLevelsForDecBlocks(block->getNextNearBlock(), path);
			CalculateLevelsForDecBlocks(block->getNextFarBlock(), path);
			path.pop_back();
		}
	};
};