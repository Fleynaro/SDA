#include "ExprUnification.h"

CE::Decompiler::Optimization::ExprUnification::ExprUnification(INode* node)
	: ExprModification(node)
{}

void CE::Decompiler::Optimization::ExprUnification::start() {
	if (auto opNode = dynamic_cast<OperationalNode*>(getNode())) {
		processOpNode(opNode);
	}
	else if (auto mirrorNode = dynamic_cast<MirrorNode*>(getNode())) {
		processMirrorNodes(mirrorNode);
	}
}

// remove MirrorNode

void CE::Decompiler::Optimization::ExprUnification::processMirrorNodes(MirrorNode* mirrorNode) {
	replace(mirrorNode->m_node);
}

// unify term order: left operand is expr, right operand is leaf (5*x -> x*5, x2 + (y6 + 1) -> (y6 + 1) + x2)

void CE::Decompiler::Optimization::ExprUnification::processOpNode(OperationalNode* opNode) {
	if (IsOperationMoving(opNode->m_operation)) {
		if (IsSwap(opNode->m_leftNode, opNode->m_rightNode)) {
			std::swap(opNode->m_leftNode, opNode->m_rightNode);
			changed();
		}
	}
}


// leafs: x, x * 5, ...

bool CE::Decompiler::Optimization::ExprUnification::IsLeaf(INode* node) {
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

bool CE::Decompiler::Optimization::ExprUnification::IsSwap(INode* leftNode, INode* rightNode) {
	return dynamic_cast<INumberLeaf*>(leftNode) && !dynamic_cast<INumberLeaf*>(rightNode) || IsLeaf(leftNode) && !IsLeaf(rightNode);
}
