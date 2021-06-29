#pragma once
#include "ExprModification.h"

namespace CE::Decompiler::Optimization
{
	//the very beigining optimization: 5*x -> x*5, 2x + (6y + 1) -> (6y + 1) + 2x
	class ExprUnification : public ExprModification
	{
	public:
		ExprUnification(INode* node)
			: ExprModification(node)
		{}

		void start() override {
			if (auto opNode = dynamic_cast<OperationalNode*>(getNode())) {
				processOpNode(opNode);
			}
			else if (auto mirrorNode = dynamic_cast<MirrorNode*>(getNode())) {
				processMirrorNodes(mirrorNode);
			}
		}
	private:
		// remove MirrorNode
		void processMirrorNodes(MirrorNode* mirrorNode) {
			replace(mirrorNode->m_node);
		}

		// unify term order: left operand is expr, right operand is leaf (5*x -> x*5, x2 + (y6 + 1) -> (y6 + 1) + x2)
		void processOpNode(OperationalNode* opNode) {
			if (IsOperationMoving(opNode->m_operation)) {
				if (IsSwap(opNode->m_leftNode, opNode->m_rightNode)) {
					std::swap(opNode->m_leftNode, opNode->m_rightNode);
					changed();
				}
			}
		}

		// leafs: x, x * 5, ...
		static bool IsLeaf(INode* node) {
			if (dynamic_cast<ILeaf*>(node))
				return true;
			if (auto opNode = dynamic_cast<OperationalNode*>(node)) {
				if (opNode->m_operation == Mul) {
					if (dynamic_cast<INumberLeaf*>(opNode->m_rightNode) && IsLeaf(opNode->m_leftNode))
						return true;
				}
			}
			return false;
		}
		
		static bool IsSwap(INode* leftNode, INode* rightNode) {
			return dynamic_cast<INumberLeaf*>(leftNode) && !dynamic_cast<INumberLeaf*>(rightNode) || IsLeaf(leftNode) && !IsLeaf(rightNode);
		}
	};
};