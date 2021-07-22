#include "SdaCodeGraph.h"

using namespace CE;
using namespace Decompiler;

SdaCodeGraph::SdaCodeGraph(DecompiledCodeGraph* decGraph)
	: m_decGraph(decGraph)
{}

SdaCodeGraph::~SdaCodeGraph() {}

DecompiledCodeGraph* SdaCodeGraph::getDecGraph() const
{
	return m_decGraph;
}

void SdaCodeGraph::gatherAllSdaSymbols(std::set<CE::Symbol::ISymbol*>& symbols) const {
	for (const auto decBlock : getDecGraph()->getDecompiledBlocks()) {
		const auto topNodes = decBlock->getAllTopNodes();
		for (const auto topNode : topNodes) {
			GatherSdaSymbols(topNode->getNode(), symbols);
		}
	}
}

std::list<CE::Symbol::ISymbol*>& SdaCodeGraph::getSdaSymbols() {
	return m_sdaSymbols;
}

void SdaCodeGraph::GatherSdaSymbols(ExprTree::INode* node, std::set<CE::Symbol::ISymbol*>& symbols) {
	node->iterateChildNodes([&](ExprTree::INode* childNode)
	{
		GatherSdaSymbols(childNode, symbols);
	});

	if (const auto sdaSymbolLeaf = dynamic_cast<ExprTree::SdaSymbolLeaf*>(node)) {
		symbols.insert(sdaSymbolLeaf->getSdaSymbol());
	}
}
