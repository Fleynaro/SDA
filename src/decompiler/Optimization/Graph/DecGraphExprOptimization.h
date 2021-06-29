#pragma once
#include "DecGraphModification.h"
#include "../ExprOptimization.h"

namespace CE::Decompiler::Optimization
{
	class GraphExprOptimization : public GraphModification
	{
	public:
		GraphExprOptimization(DecompiledCodeGraph* decGraph)
			: GraphModification(decGraph)
		{}

		void start() override {
			passAllTopNodes([&](DecBlock::BlockTopNode* topNode) {
				optimize(topNode);
				});
		}
	private:
		// optimize expression on the specified {topNode}
		void optimize(DecBlock::BlockTopNode* topNode) {
			INode::UpdateDebugInfo(topNode->getNode());
			ExprOptimization exprOptimization(topNode);
			exprOptimization.start();
		}
	};
};