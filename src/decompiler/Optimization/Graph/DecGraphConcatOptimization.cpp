#include "DecGraphConcatOptimization.h"

CE::Decompiler::Optimization::GraphConcatOptimization::GraphConcatOptimization(DecompiledCodeGraph* decGraph)
	: GraphModification(decGraph)
{}

void CE::Decompiler::Optimization::GraphConcatOptimization::start() {
	passAllTopNodes([&](DecBlock::BlockTopNode* topNode) {
		auto node = topNode->getNode();
		INode::UpdateDebugInfo(node);
		findConcat(node);
		});
}

void CE::Decompiler::Optimization::GraphConcatOptimization::findConcat(INode* node) {
	node->iterateChildNodes([&](INode* childNode) {
		findConcat(childNode);
		});

	if (auto opNode = dynamic_cast<OperationalNode*>(node)) {
		if (opNode->m_operation == Concat) {

		}
	}
}
