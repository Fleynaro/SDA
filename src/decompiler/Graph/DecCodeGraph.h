#pragma once
#include "DecCodeGraphBlock.h"
#include "DecPCodeGraph.h"

namespace CE::Decompiler
{
	class DecompiledCodeGraph
	{
		FunctionPCodeGraph* m_funcGraph;
		std::list<DecBlock*> m_decompiledBlocks;
		std::list<DecBlock*> m_removedDecompiledBlocks;
		std::list<Symbol::Symbol*> m_symbols;

		// for pcode emulator
		struct SymbolValue
		{
			Storage m_storage;
			bool m_after;
		};
		std::map<ComplexOffset, std::map<Symbol::Symbol*, SymbolValue>> m_symbolValues;
		std::map<ComplexOffset, int> m_stackPointerValues;
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

		void removeNotUsedSymbols();

		DecBlock::BlockTopNode* findBlockTopNodeAtOffset(ComplexOffset offset);

		// for pcode emulator
		void addSymbolValue(ComplexOffset offset, Symbol::Symbol* symbol, const Storage& storage = Storage(), bool after = true);

		// for pcode emulator
		std::map<ComplexOffset, std::map<Symbol::Symbol*, SymbolValue>>& getSymbolValues();

		// for pcode emulator
		std::map<ComplexOffset, int>& getStackPointerValues();

		int getStackPointerValueAtOffset(ComplexOffset offset);

		// recalculate levels because some blocks can be removed (while parsing AND/OR block constructions)
		void recalculateLevelsForBlocks();

		// calculate count of lines(height) for each block beginining from lower blocks (need as some score for linearization)
		static int CalculateHeightForDecBlocks(DecBlock* block);
	private:

		// pass decompiled graph and calculate max distance from the root to each node (dec block). Similarly to asm graph!
		static void CalculateLevelsForDecBlocks(DecBlock* block, std::list<DecBlock*>& path);
	};
};