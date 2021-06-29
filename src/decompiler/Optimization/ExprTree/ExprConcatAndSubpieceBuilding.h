#pragma once
#include "ExprModification.h"

namespace CE::Decompiler::Optimization
{
	//делать в самом конце. сюда же построение subpiece(если param1 - 8-байтовый объект). маску вычислять без кеша также. сделать linearexpr для логических операций.
	class ExprConcatAndSubpieceBuilding : public ExprModification
	{
	public:
		ExprConcatAndSubpieceBuilding(INode* node)
			: ExprModification(node)
		{}

		void start() override {
			dispatch(getNode());
		}
	private:
		void dispatch(INode* node) {
			if (auto opNode = dynamic_cast<OperationalNode*>(node)) {
				if (opNode->m_operation == Or) {
					processOpNode(opNode);
				}
			}
		}

		void processOpNode(OperationalNode* opNode) {
			auto pairOp1 = GetConcatOperand(opNode->m_rightNode);
			std::pair<INode*, int> pairOp2;
			auto leftOpNode = dynamic_cast<OperationalNode*>(opNode->m_leftNode);
			INode* leftTail = nullptr;
			if (leftOpNode && leftOpNode->m_operation == Or) {
				pairOp2 = GetConcatOperand(leftOpNode->m_rightNode);
				leftTail = leftOpNode->m_leftNode;
			}
			else {
				pairOp2 = GetConcatOperand(opNode->m_leftNode);
			}

			if (pairOp1.second || pairOp2.second) {
				if (pairOp2.second < pairOp1.second)
					std::swap(pairOp1, pairOp2);
				if (pairOp2.second - pairOp1.second == pairOp1.first->getSize() * 0x8) {
					auto sumSize = pairOp1.first->getSize() + pairOp2.first->getSize();
					auto newNode = new OperationalNode(pairOp2.first, pairOp1.first, Concat);
					if (pairOp1.second)
						newNode = new OperationalNode(newNode, new NumberLeaf((uint64_t)pairOp1.second, 4), Shl);
					if (leftTail) {
						newNode = new OperationalNode(leftTail, newNode, Or);
					}
					newNode->m_instr = opNode->m_instr;
					replace(newNode);
				}
			}
		}

		static std::pair<INode*, int> GetConcatOperand(INode* node) {
			if (auto curExpr = dynamic_cast<OperationalNode*>(node)) {
				if (curExpr->m_operation == Shl) {
					if (auto shlNumberLeaf = dynamic_cast<INumberLeaf*>(curExpr->m_rightNode)) {
						return std::make_pair(curExpr->m_leftNode, (int)shlNumberLeaf->getValue());
					}
				}
			}
			return std::make_pair(node, 0x0);
		}
	};
};