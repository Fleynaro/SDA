#pragma once
#include <decompiler/ExprTree/ExprTree.h>
#include <decompiler/DecTopNode.h>

namespace CE::Decompiler::Optimization
{
	using namespace ExprTree;
	class ExprOptimization
	{
		TopNode* m_topNode;
		bool m_isChanged = false;
	public:
		ExprOptimization(TopNode* node);

		void start();

	private:
		void optimizeGenerally(INode* node);

		void linearExprToOpNodes(INode* node);

		void opNodesToLinearExpr(INode* node);
	};
};