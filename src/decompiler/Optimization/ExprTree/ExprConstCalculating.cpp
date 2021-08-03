#include "ExprConstCalculating.h"

using namespace CE::Decompiler;

//a << 0x2{2} => a * 4

Optimization::ExprConstCalculating::ExprConstCalculating(OperationalNode* opNode)
	: ExprModification(opNode)
{}

// make calculation of two constant operands ({size} need for float/double operations)

uint64_t Optimization::ExprConstCalculating::Calculate(uint64_t op1, uint64_t op2, OperationType operation, int size) {
	switch (operation)
	{
	case fAdd: {
		if (size == 0x4)
		{
			auto result = (float&)op1 + (float&)op2;
			return (uint32_t&)result;
		}
		auto result = (double&)op1 + (double&)op2;
		return (uint64_t&)result;
	}
	case fMul: {
		if (size == 0x4)
		{
			auto result = (float&)op1 * (float&)op2;
			return (uint32_t&)result;
		}
		auto result = (double&)op1 * (double&)op2;
		return (uint64_t&)result;
	}
	case fDiv: {
		if (size == 0x4)
		{
			auto result = (float&)op1 / (float&)op2;
			return (uint32_t&)result;
		}
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

void Optimization::ExprConstCalculating::start() {
	if (IsOperationUnsupportedToCalculate(getOpNode()->m_operation))
		return;
	// try to apply different kinds of operations
	if (!processConstOperands())
		if (!processConstRightOperand())
			if (!processEqualOperands())
				if (!processShl())
				{
				}
}

OperationalNode* Optimization::ExprConstCalculating::getOpNode() {
	return dynamic_cast<OperationalNode*>(getNode());
}

//5 + 2 => 7

bool Optimization::ExprConstCalculating::processConstOperands() {
	const auto opNode = getOpNode();
	if (auto leftNumberLeaf = dynamic_cast<INumberLeaf*>(opNode->m_leftNode)) {
		if (auto rightNumberLeaf = dynamic_cast<INumberLeaf*>(opNode->m_rightNode)) {
			const auto result = Calculate(leftNumberLeaf->getValue(), rightNumberLeaf->getValue(), opNode->m_operation);
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

bool Optimization::ExprConstCalculating::processConstRightOperand() {
	const auto opNode = getOpNode();
	if (auto rightNumberLeaf = dynamic_cast<INumberLeaf*>(opNode->m_rightNode)) {
		const auto opNodeMask = CalculateMask(opNode);
		if (opNodeMask.isZero()) {
			replace(new NumberLeaf(static_cast<uint64_t>(0), opNode->getSize()));
			return true;
		}
		
		if (opNode->m_operation != Div && opNode->m_operation != Mod) {
			if (rightNumberLeaf->getValue() == 0) {
				// [var_2_32] * 0
				if (opNode->m_operation == Mul || opNode->m_operation == And) {
					replace(new NumberLeaf(static_cast<uint64_t>(0), opNode->getSize()));
					return true;
				}
				// todo: add instructions from opNode to opNode->m_leftNode
				replace(opNode->m_leftNode);
				return true;
			}
			if (opNode->m_operation == Or) {
				// [var_2_32] | 0xFFFFFFFF
				if ((rightNumberLeaf->getValue() | opNodeMask.getValue()) == rightNumberLeaf->getValue()) {
					replace(rightNumberLeaf);
					return true;
				}
			}
			else if (opNode->m_operation == And) {
				// [var_2_32] & 0xFFFFFFFF
				const auto leftNodeMask = CalculateMask(opNode->m_leftNode);
				if ((leftNodeMask.getValue() & rightNumberLeaf->getValue()) == leftNodeMask.getValue()) {
					const auto newExpr = opNode->m_leftNode;
					replace(newExpr);
					return true;
				}
			}
		}
		else {
			// [var_2_32] * 1
			if (rightNumberLeaf->getValue() == 1) {
				const auto newExpr = opNode->m_leftNode;
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

bool Optimization::ExprConstCalculating::processEqualOperands() {
	const auto opNode = getOpNode();
	if (opNode->m_operation == Xor || opNode->m_operation == And || opNode->m_operation == Or) {
		if (opNode->m_leftNode->getHash().getHashValue() == opNode->m_rightNode->getHash().getHashValue()) {
			if (opNode->m_operation == Xor) {
				//[sym1] ^ [sym1] => 0x0
				replace(new NumberLeaf(static_cast<uint64_t>(0), opNode->getSize()));
				return true;
			}
			const auto newExpr = opNode->m_leftNode;
			replace(newExpr);
			return true;
		}
	}
	return false;
}

//a << 0x2{2} => a * 4

bool Optimization::ExprConstCalculating::processShl() {
	const auto opNode = getOpNode();
	if (opNode->m_operation == Shl) {
		if (auto numberLeaf = dynamic_cast<INumberLeaf*>(opNode->m_rightNode)) {
			const auto value = numberLeaf->getValue();
			if (value >= 1 && value <= 3) {
				opNode->m_operation = Mul;
				numberLeaf->setValue(static_cast<uint64_t>(1) << value);
				return true;
			}
		}
	}
	return false;
}
