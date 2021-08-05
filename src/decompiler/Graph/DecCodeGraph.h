#pragma once
#include "DecCodeGraphBlock.h"
#include "DecPCodeGraph.h"

namespace CE::Decompiler
{
	class DecompiledCodeGraph
	{
	public:
		DecompiledCodeGraph(FunctionPCodeGraph* funcGraph);

		~DecompiledCodeGraph();

		FunctionPCodeGraph* getFuncGraph() const;

		DecBlock* getStartBlock();

		std::list<DecBlock*>& getDecompiledBlocks();

		std::list<Symbol::Symbol*>& getSymbols();

		void removeDecompiledBlock(DecBlock* decBlock);

		void addSymbol(Symbol::Symbol* symbol);

		void removeSymbol(Symbol::Symbol* symbol);

		void cloneAllExpr();

		void sortBlocksByLevel();

		void checkOnSingleParents();

		HS getHash();

		DecBlock::BlockTopNode* findBlockTopNodeByOffset(ComplexOffset offset);

		// recalculate levels because some blocks can be removed (while parsing AND/OR block constructions)
		void recalculateLevelsForBlocks();

		// calculate count of lines(height) for each block beginining from lower blocks (need as some score for linearization)
		static int CalculateHeightForDecBlocks(DecBlock* block);
	private:
		FunctionPCodeGraph* m_funcGraph;
		std::list<DecBlock*> m_decompiledBlocks;
		std::list<DecBlock*> m_removedDecompiledBlocks;
		std::list<Symbol::Symbol*> m_symbols;

		// pass decompiled graph and calculate max distance from the root to each node (dec block). Similarly to asm graph!
		static void CalculateLevelsForDecBlocks(DecBlock* block, std::list<DecBlock*>& path);
	};
};