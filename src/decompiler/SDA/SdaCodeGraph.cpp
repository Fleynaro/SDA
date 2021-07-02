#include "SdaCodeGraph.h"

using namespace CE;
using namespace CE::Decompiler;

CE::Decompiler::SdaCodeGraph::SdaCodeGraph(DecompiledCodeGraph* decGraph)
	: m_decGraph(decGraph)
{}

CE::Decompiler::SdaCodeGraph::~SdaCodeGraph() {
}

DecompiledCodeGraph* CE::Decompiler::SdaCodeGraph::getDecGraph() {
	return m_decGraph;
}

std::list<CE::Symbol::ISymbol*>& CE::Decompiler::SdaCodeGraph::getSdaSymbols() {
	return m_sdaSymbols;
}
