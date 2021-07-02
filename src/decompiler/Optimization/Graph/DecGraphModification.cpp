#include "DecGraphModification.h"

using namespace CE::Decompiler;

// iterate over all top nodes of the dec. graph (allow to access all expressions)

CE::Decompiler::GraphModification::GraphModification(DecompiledCodeGraph* decGraph)
	: m_decGraph(decGraph)
{}

void CE::Decompiler::GraphModification::passAllTopNodes(std::function<void(DecBlock::BlockTopNode*)> func) {
	for (const auto decBlock : m_decGraph->getDecompiledBlocks()) {
		for (auto topNode : decBlock->getAllTopNodes()) {
			func(topNode);
		}
	}
}
