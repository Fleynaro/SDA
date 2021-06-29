#pragma once
#include "ExprModification.h"
#include "ExprConstCalculating.h"

namespace CE::Decompiler::Optimization
{
	// Simplify simple condition: SBORROW, rax + -0x2 < 0 -> rax < -0x2 * -1
	class ExprSimpleConditionOptimization : public ExprModification
	{
	public:
		ExprSimpleConditionOptimization(Condition* cond)
			: ExprModification(cond)
		{}

		void start() override {
			processSBORROW(getCondition());
			while (moveTermToRightPartOfCondition(getCondition())) {
				OptimizeNode(getCondition()->m_rightNode);
			}
		}

		Condition* getCondition() {
			return dynamic_cast<Condition*>(getNode());
		}
	private:
		//replace SBORROW condition with normal
		//SBORROW(*(uint_32t*)([reg_rsp_64]), 0x4{4}) == ((*(uint_32t*)([reg_rsp_64]) + 0x3fffffffc{-4}) < 0x0{0}))
		bool processSBORROW(Condition* cond) {
			if (auto func = dynamic_cast<FunctionalNode*>(cond->m_leftNode)) {
				if (func->m_funcId == FunctionalNode::Id::SBORROW && (cond->m_cond == Condition::Eq || cond->m_cond == Condition::Ne)) {
					if (auto mainCond = dynamic_cast<Condition*>(cond->m_rightNode)) {
						if (mainCond->m_cond == Condition::Lt) {
							auto newCondType = Condition::Ge;
							if (cond->m_cond == Condition::Ne)
								newCondType = Condition::Lt;
							auto newCond = new Condition(func->m_leftNode, func->m_rightNode, newCondType, cond->m_instr);
							replace(newCond);
							return true;
						}
					}
				}
			}
			return false;
		}

		//rax + -0x2 < 0 -> rax < -0x2 * -1
		//if(((((([mem_2_32] *.4 0x4{4}) >>.4 0x2{2}) *.4 0xffffffff{-1}) +.4 [mem_3_32]) == 0x0{0})) -> if(([mem_3_32] == ((([mem_2_32] *.4 0x4{4}) >>.4 0x2{2}) *.4 0x1{1})))
		bool moveTermToRightPartOfCondition(Condition* cond) {
			if (auto addOpNode = dynamic_cast<OperationalNode*>(cond->m_leftNode)) {
				if (addOpNode->m_operation == Add) {
					auto leftNode = addOpNode->m_leftNode;
					auto rightNode = addOpNode->m_rightNode;
					bool isTermMoving = false;
					if (dynamic_cast<INumberLeaf*>(addOpNode->m_rightNode) || IsNegative(addOpNode->m_rightNode, addOpNode->getSize())) {
						isTermMoving = true;
					}
					else if (IsNegative(addOpNode->m_leftNode, addOpNode->getSize())) {
						std::swap(leftNode, rightNode);
						isTermMoving = true;
					}

					if (isTermMoving) {
						//move expr from left node of the condition to the right node being multiplied -1
						auto newPartOfRightExpr = new OperationalNode(rightNode, new NumberLeaf((uint64_t)(int64_t)-1, addOpNode->getSize()), Mul);
						auto newRightExpr = new OperationalNode(cond->m_rightNode, newPartOfRightExpr, Add);
						auto newCond = new Condition(leftNode, newRightExpr, cond->m_cond, cond->m_instr);
						replace(newCond);
						return true;
					}
				}
			}
			return false;
		}

		static void OptimizeNode(INode* node) {
			node->iterateChildNodes(OptimizeNode);
			if (auto opNode = dynamic_cast<OperationalNode*>(node)) {
				ExprConstCalculating exprConstCalculating(opNode);
				exprConstCalculating.start();
			}
		}

		//check negative arithmetic sign of expr node
		static bool IsNegative(INode* node, int size) {
			if (auto numberLeaf = dynamic_cast<INumberLeaf*>(node)) {
				if ((numberLeaf->getValue() >> (size * 0x8 - 1)) & 0b1)
					return true;
			}
			else if (auto opNode = dynamic_cast<OperationalNode*>(node)) {
				if (opNode->m_operation == Mul)
					return IsNegative(opNode->m_rightNode, size);
			}
			return false;
		}
	};
};