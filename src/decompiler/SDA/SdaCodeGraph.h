#pragma once
#include "../Graph/DecCodeGraph.h"
#include "ExprTree/ExprTreeSda.h"
#include "../ExprTree/ExprTree.h"

namespace CE::Decompiler
{
	// Based on decompiled graph with high-level sda symbols
	class SdaCodeGraph
	{
	public:
		SdaCodeGraph(DecompiledCodeGraph* decGraph)
			: m_decGraph(decGraph)
		{}

		~SdaCodeGraph() {
		}

		DecompiledCodeGraph* getDecGraph() {
			return m_decGraph;
		}

		/*CE::Symbol::AbstractSymbol* findSdaSymbolByName(std::string name) {
			for (auto symbol : m_sdaSymbols) {
				if (name == symbol->getName())
					return symbol;
			}
			return nullptr;
		}*/

		std::list<CE::Symbol::ISymbol*>& getSdaSymbols() {
			return m_sdaSymbols;
		}
	private:
		DecompiledCodeGraph* m_decGraph;
		std::list<CE::Symbol::ISymbol*> m_sdaSymbols;
	};

};