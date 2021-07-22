#pragma once
#include "ExprTree/ExprTreeSda.h"
#include <decompiler/Graph/DecCodeGraph.h>
#include <decompiler/ExprTree/ExprTree.h>

namespace CE::Decompiler
{
	// Based on decompiled graph with high-level sda symbols
	class SdaCodeGraph
	{
	public:
		SdaCodeGraph(DecompiledCodeGraph* decGraph);

		~SdaCodeGraph();

		DecompiledCodeGraph* getDecGraph() const;

		void gatherAllSdaSymbols(std::set<CE::Symbol::ISymbol*>& symbols) const;

		std::list<CE::Symbol::ISymbol*>& getSdaSymbols();
	private:
		DecompiledCodeGraph* m_decGraph;
		std::list<CE::Symbol::ISymbol*> m_sdaSymbols;

		static void GatherSdaSymbols(ExprTree::INode* node, std::set<CE::Symbol::ISymbol*>& symbols);
	};

};