#pragma once
#include "ExprModification.h"

namespace CE::Decompiler::Optimization
{
	// Make arithmetic/logic simplification: 5 + 2 => 7, x * 0 -> 0, x & x => x
	class ExprConstCalculating : public ExprModification
	{
	public:
		ExprConstCalculating(OperationalNode* opNode)
			: ExprModification(opNode)
		{}

		// make calculation of two constant operands ({size} need for float/double operations)
		static uint64_t Calculate(uint64_t op1, uint64_t op2, OperationType operation, int size = 0x8) {
			switch (operation)
			{
			case fAdd:
				if (size == 0x4)
				{
					auto result = (float&)op1 + (float&)op2;
					return (uint32_t&)result;
				}
				else {
					auto result = (double&)op1 + (double&)op2;
					return (uint64_t&)result;
				}
			case fMul:
				if (size == 0x4)
				{
					auto result = (float&)op1 * (float&)op2;
					return (uint32_t&)result;
				}
				else {
					auto result = (double&)op1 * (double&)op2;
					return (uint64_t&)result;
				}
			case fDiv:
				if (size == 0x4)
				{
					auto result = (float&)op1 / (float&)op2;
					return (uint32_t&)result;
				}
				else {
					auto result = (double&)op1 / (double&)op2;
					return (uint64_t&)result;
				}
			case Add:
				return op1 + op2;
			case Mul:
				return op1 * op2;
			case Div:
				return op1 / op2;
			case Mod:
				return op1 % op2;
			case And:
				return op1 & op2;
			case Or:
				return op1 | op2;
			case Xor:
				return op1 ^ op2;
			case Shr:
				return op1 >> op2;
			case Shl:
				return op1 << op2;
			}
			return 0;
		}

		void start() override {
			auto opNode = getOpNode();
			if (IsOperationUnsupportedToCalculate(opNode->m_operation))
				return;
			// try to apply different kinds of operations
			if (!processConstOperands(opNode))
				if (!processConstRightOperand(opNode))
					if (!processEqualOperands(opNode))
						if (!processShl(opNode))
						{}
		}

		OperationalNode* getOpNode() {
			return dynamic_cast<OperationalNode*>(getNode());
		}
	private:
		//5 + 2 => 7
		bool processConstOperands(OperationalNode* opNode) {
			if (auto leftNumberLeaf = dynamic_cast<INumberLeaf*>(opNode->m_leftNode)) {
				if (auto rightNumberLeaf = dynamic_cast<INumberLeaf*>(opNode->m_rightNode)) {
					auto result = Calculate(leftNumberLeaf->getValue(), rightNumberLeaf->getValue(), opNode->m_operation);
					replace(new NumberLeaf(result, opNode->getSize()));
					return true;
				}
			}
			return false;
		}

		//[var_2_32] * 0							=>		0
		//[var_2_32] ^ [var_2_32]					=>		0
		//[var_2_32] + 0							=>		[var_2_32]
		//[var_2_32] * 1							=>		[var_2_32]
		//[var_2_32] & 0xffffffff00000000{0}		=>		0x0
		//[var_2_32] & 0xffffffff{-1}				=>		[var_2_32]	
		bool processConstRightOperand(OperationalNode* opNode) {
			if (auto rightNumberLeaf = dynamic_cast<INumberLeaf*>(opNode->m_rightNode)) {
				if (opNode->m_operation != Div && opNode->m_operation != Mod) {
					auto opNodeMask = CalculateMask(opNode);
					if (opNodeMask.isZero()) {
						replace(new NumberLeaf((uint64_t)0, opNode->getSize()));
						return true;
					}

					if (rightNumberLeaf->getValue() == 0) {
						if (opNode->m_operation == Mul || opNode->m_operation == And) {
							replace(new NumberLeaf((uint64_t)0, opNode->getSize()));
							return true;
						}
						else {
							auto newExpr = opNode->m_leftNode;
							replace(opNode->m_leftNode);
							return true;
						}
					}
					else {
						if (opNode->m_operation == Or) {
							if ((rightNumberLeaf->getValue() | opNodeMask.getValue()) == rightNumberLeaf->getValue()) {
								replace(rightNumberLeaf);
								return true;
							}
						} else if (opNode->m_operation == And) {
							auto leftNodeMask = CalculateMask(opNode->m_leftNode);
							if ((leftNodeMask.getValue() & rightNumberLeaf->getValue()) == leftNodeMask.getValue()) {
								auto newExpr = opNode->m_leftNode;
								replace(newExpr);
								return true;
							}
						}
					}
				}
				else {
					if (rightNumberLeaf->getValue() == 1) {
						auto newExpr = opNode->m_leftNode;
						replace(newExpr);
						return true;
					}
				}
			}
			return false;
		}

		//[sym1] & [sym1] => [sym1]
		//[sym1] | [sym1] => [sym1]
		//[sym1] ^ [sym1] => 0x0
		bool processEqualOperands(OperationalNode* opNode) {
			if (opNode->m_operation == Xor || opNode->m_operation == And || opNode->m_operation == Or) {
				if (opNode->m_leftNode->getHash().getHashValue() == opNode->m_rightNode->getHash().getHashValue()) {
					if (opNode->m_operation == Xor) {
						replace(new NumberLeaf((uint64_t)0, opNode->getSize()));
						return true;
					}
					else {
						auto newExpr = opNode->m_leftNode;
						replace(newExpr);
						return true;
					}
				}
			}
			return false;
		}

		//a << 0x2{2} => a * 4
		bool processShl(OperationalNode* opNode) {
			if (opNode->m_operation == Shl) {
				if (auto numberLeaf = dynamic_cast<INumberLeaf*>(opNode->m_rightNode)) {
					auto value = numberLeaf->getValue();
					if (value >= 1 && value <= 3) {
						opNode->m_operation = Mul;
						numberLeaf->setValue((uint64_t)1 << value);
						return true;
					}
				}
			}
			return false;
		}
	};
};