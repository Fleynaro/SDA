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

		DecompiledCodeGraph* getDecGraph();

		/*CE::Symbol::AbstractSymbol* findSdaSymbolByName(std::string name) {
			for (auto symbol : m_sdaSymbols) {
				if (name == symbol->getName())
					return symbol;
			}
			return nullptr;
		}*/

		std::list<CE::Symbol::ISymbol*>& getSdaSymbols();
	private:
		DecompiledCodeGraph* m_decGraph;
		std::list<CE::Symbol::ISymbol*> m_sdaSymbols;
	};

};