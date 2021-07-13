#include "ExprExpandingToLinearExpr.h"

using namespace CE::Decompiler;

// arithmetic/logic/floating operation state

Optimization::ExprExpandingToLinearExpr::ExprExpandingToLinearExpr(OperationalNode* node)
	: ExprModification(node)
{}

void Optimization::ExprExpandingToLinearExpr::start() {
	if (!defineOperationState(getOpNode()->m_operation))
		return;
	// find all terms and know if m_doBuilding is true
	defineTerms(getOpNode(), m_invisibleMultiplier);
	if (m_doBuilding) {
		const auto linearExpr = buildLinearExpr();
		replace(linearExpr);
		/*if (m_terms.size() == 1 && m_constTerm == 0x0) {
		auto baseTerm = m_terms.begin()->second.first;
		if (baseTerm != getNode())
		replace(baseTerm);
		}*/
	}
}

OperationalNode* Optimization::ExprExpandingToLinearExpr::getOpNode() {
	return dynamic_cast<OperationalNode*>(getNode());
}

// using terms (including constant term) build linear expression

LinearExpr* Optimization::ExprExpandingToLinearExpr::buildLinearExpr() {
	const auto constTerm = new NumberLeaf((uint64_t&)m_constTerm, m_constTermSize);
	auto linearExpr = new LinearExpr(constTerm, m_operationAdd); // todo: change size for number
																 // iterate over all terms
	for (auto termInfo : m_terms) {
		auto node = termInfo.second.first;
		auto mask = CalculateMask(node);
		const auto multiplier = (uint64_t&)termInfo.second.second;

		INode* term;
		if ((multiplier & mask.getValue()) == (m_invisibleMultiplier & mask.getValue())) {
			term = termInfo.second.first;
		}
		else {
			const auto multiplierLeaf = new NumberLeaf(multiplier, node->getSize());
			term = new OperationalNode(node, multiplierLeaf, m_operationMul);
		}
		linearExpr->addTerm(term);
	}
	return linearExpr;
}

//(5x - 10y) * 2 + 5 ->	{x: 10, y: -20, constTerm: 5}

void Optimization::ExprExpandingToLinearExpr::defineTerms(INode* node, int64_t k, int level) {
	const auto size = node->getSize();

	// called for add operation (e.g. x + 5)
	if (auto numberLeaf = dynamic_cast<INumberLeaf*>(node)) {
		const auto constTerm = ExprConstCalculating::Calculate(
			numberLeaf->getValue(),
			k,
			m_operationMul,
			size
		);

		m_constTerm = ExprConstCalculating::Calculate(
			m_constTerm,
			constTerm,
			m_operationAdd,
			size
		);
		m_constTermSize = std::max(m_constTermSize, size);
		m_doBuilding = true;
		return;
	}

	// if the source expression is enough long, in other words, can be presented as linear expr.
	if (level == 2) {
		m_doBuilding = true;
	}

	//(x * 2) * 3 => x * 6
	if (const auto opNode = dynamic_cast<OperationalNode*>(node)) {
		if (opNode->m_operation == m_operationAdd) {
			defineTerms(opNode->m_leftNode, k, level + 1);
			defineTerms(opNode->m_rightNode, k, level + 1);
			return;
		}
		else if (opNode->m_operation == m_operationMul) {
			if (auto rightNumberLeaf = dynamic_cast<INumberLeaf*>(opNode->m_rightNode)) {
				const auto newK = ExprConstCalculating::Calculate( // e.g. 5x * 5 -> 25x
					rightNumberLeaf->getValue(),
					k,
					m_operationMul,
					size
				);
				defineTerms(opNode->m_leftNode, newK, level + 1);
				return;
			}
		}
	}

	// if {node} is not OperationalNode then it is a term
	const auto hashVal = node->getHash().getHashValue();
	if (m_terms.find(hashVal) == m_terms.end()) {
		m_terms[hashVal] = std::make_pair(node, 0);
	}
	auto newK = ExprConstCalculating::Calculate(
		m_terms[hashVal].second,
		k,
		m_operationAdd,
		size
	);
	m_terms[hashVal] = std::make_pair(node, newK);
}

bool Optimization::ExprExpandingToLinearExpr::defineOperationState(OperationType op) {
	if (op == Add || op == Mul) {
		m_operationAdd = Add;
		m_operationMul = Mul;
		m_invisibleMultiplier = 1;
		return true;
	}
	if (op == Or || op == And) {
		m_operationAdd = Or;
		m_operationMul = And;
		m_invisibleMultiplier = static_cast<int64_t>(-1);
		return true;
	}

	//TODO: for float and double
	//if (op == fAdd || op == fMul) {
	//	m_operationAdd = fAdd;
	//	m_operationMul = fMul;
	//	m_invisibleMultiplier = 0;
	//	return true;
	//}
	return false;
}
