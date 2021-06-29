#pragma once
#include "DecGraphModification.h"

namespace CE::Decompiler::Optimization
{
	class GraphConcatOptimization : public GraphModification
	{
	public:
		GraphConcatOptimization(DecompiledCodeGraph* decGraph)
			: GraphModification(decGraph)
		{}

		void start() override {
			passAllTopNodes([&](DecBlock::BlockTopNode* topNode) {
				auto node = topNode->getNode();
				INode::UpdateDebugInfo(node);
				findConcat(node);
				});
		}
	private:
		void findConcat(INode* node) {
			node->iterateChildNodes([&](INode* childNode) {
				findConcat(childNode);
				});

			if (auto opNode = dynamic_cast<OperationalNode*>(node)) {
				if (opNode->m_operation == Concat) {

				}
			}
		}
	};
};