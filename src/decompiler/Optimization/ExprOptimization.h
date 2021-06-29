#pragma once
#include "ExprTree/ExprUnification.h"
#include "ExprTree/ExprConstCalculating.h"
#include "ExprTree/ExprExpandingToLinearExpr.h"
#include "ExprTree/ExprConcatAndSubpieceBuilding.h"
#include "ExprTree/ExprSimpleCondOptimization.h"
#include "ExprTree/ExprCompositeCondOptimization.h"
#include "ExprTree/ExprConstCondCalculating.h"

namespace CE::Decompiler::Optimization
{
	using namespace ExprTree;
	class ExprOptimization
	{
		TopNode* m_topNode;
		bool m_isChanged = false;
	public:
		ExprOptimization(TopNode* node)
			: m_topNode(node)
		{}

		void start() {
			linearExprToOpNodes(m_topNode->getNode()); // remove linear expressions

			// todo: calculate hashes using cache
			do {
				m_isChanged = false;
				optimizeGenerally(m_topNode->getNode());
			} while (m_isChanged);

			opNodesToLinearExpr(m_topNode->getNode()); // create linear expressions
		}

	private:
		void optimizeGenerally(INode* node) {
			node->iterateChildNodes([&](INode* childNode) {
				optimizeGenerally(childNode);
				});

			ExprUnification exprUnification(node);
			exprUnification.start();
			node = exprUnification.getNode();
			m_isChanged |= exprUnification.isChanged();
			
			if (auto opNode = dynamic_cast<OperationalNode*>(node)) {
				ExprConstCalculating exprConstCalculating(opNode);
				exprConstCalculating.start();
				node = exprConstCalculating.getNode();
				m_isChanged |= exprConstCalculating.isChanged();
			}

			ExprConcatAndSubpieceBuilding exprConcatAndSubpieceBuilding(node);
			exprConcatAndSubpieceBuilding.start();
			node = exprConcatAndSubpieceBuilding.getNode();
			m_isChanged |= exprConcatAndSubpieceBuilding.isChanged();

			// condition
			if (auto cond = dynamic_cast<AbstractCondition*>(node)) {
				ExprConstConditionCalculating exprConstConditionCalculating(cond);
				exprConstConditionCalculating.start();
				node = exprConstConditionCalculating.getNode();
				m_isChanged |= exprConstConditionCalculating.isChanged();
			}
			if (auto cond = dynamic_cast<Condition*>(node)) {
				ExprSimpleConditionOptimization exprSimpleConditionOptimization(cond);
				exprSimpleConditionOptimization.start();
				node = exprSimpleConditionOptimization.getNode();
				m_isChanged |= exprSimpleConditionOptimization.isChanged();
			}
			else if (auto compCond = dynamic_cast<CompositeCondition*>(node)) {
				ExprCompositeConditionOptimization exprCompositeConditionOptimization(compCond);
				exprCompositeConditionOptimization.start();
				node = exprCompositeConditionOptimization.getNode();
				m_isChanged |= exprCompositeConditionOptimization.isChanged();
			}
		}

		void linearExprToOpNodes(INode* node) {
			node->iterateChildNodes([&](INode* childNode) {
				linearExprToOpNodes(childNode);
				});

			if (auto linearExpr = dynamic_cast<LinearExpr*>(node)) {
				node = linearExpr->getConstTerm();
				for (auto term : linearExpr->getTerms()) {
					node = new OperationalNode(node, term, linearExpr->m_operation);
				}
				linearExpr->replaceWith(node);
				delete linearExpr;
			}
		}

		void opNodesToLinearExpr(INode* node) {
			if (auto opNode = dynamic_cast<OperationalNode*>(node)) {
				ExprExpandingToLinearExpr exprExpandingToLinearExpr(opNode);
				exprExpandingToLinearExpr.start();
				node = exprExpandingToLinearExpr.getNode();
			}

			node->iterateChildNodes([&](INode* childNode) {
				opNodesToLinearExpr(childNode);
				});
		}
	};
};