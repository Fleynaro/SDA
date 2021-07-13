#include "SdaCodeGraph.h"

using namespace CE;
using namespace Decompiler;

SdaCodeGraph::SdaCodeGraph(DecompiledCodeGraph* decGraph)
	: m_decGraph(decGraph)
{}

SdaCodeGraph::~SdaCodeGraph() {
}

DecompiledCodeGraph* SdaCodeGraph::getDecGraph() const
{
	return m_decGraph;
}

std::list<CE::Symbol::ISymbol*>& SdaCodeGraph::getSdaSymbols() {
	return m_sdaSymbols;
}
