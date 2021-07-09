#include "ExprTreeLinearExpr.h"

using namespace CE::Decompiler;
using namespace CE::Decompiler::ExprTree;

CE::Decompiler::ExprTree::LinearExpr::LinearExpr(INumberLeaf* constTerm, OperationType operation)
	: m_constTerm(constTerm), m_operation(operation)
{
	m_constTerm->addParentNode(this);
}

CE::Decompiler::ExprTree::LinearExpr::~LinearExpr() {
	for (auto term : m_terms) {
		term->removeBy(this);
	}
	m_constTerm->removeBy(this);
}

void CE::Decompiler::ExprTree::LinearExpr::addTerm(ExprTree::INode* term) {
	term->addParentNode(this);
	m_terms.push_back(term);
}

void CE::Decompiler::ExprTree::LinearExpr::setConstTermValue(int64_t constTerm) const
{
	m_constTerm->setValue(static_cast<uint64_t>(constTerm));
}

int64_t CE::Decompiler::ExprTree::LinearExpr::getConstTermValue() const
{
	return static_cast<int64_t>(m_constTerm->getValue());
}

std::list<ExprTree::INode*>& CE::Decompiler::ExprTree::LinearExpr::getTerms() {
	return m_terms;
}

INumberLeaf* CE::Decompiler::ExprTree::LinearExpr::getConstTerm() const
{
	return m_constTerm;
}

void CE::Decompiler::ExprTree::LinearExpr::replaceNode(ExprTree::INode* node, ExprTree::INode* newNode) {
	for (auto it = m_terms.begin(); it != m_terms.end(); it++) {
		if (node == *it) {
			*it = newNode;
		}
	}
	if (node == m_constTerm)
		m_constTerm = dynamic_cast<INumberLeaf*>(newNode);
}

std::list<INode*> CE::Decompiler::ExprTree::LinearExpr::getNodesList() {
	auto list = m_terms;
	list.push_back(m_constTerm);
	return list;
}

std::list<PCode::Instruction*> CE::Decompiler::ExprTree::LinearExpr::getInstructionsRelatedTo() {
	return {};
}

int CE::Decompiler::ExprTree::LinearExpr::getSize() {
	auto size = 1;
	for (auto term : m_terms) {
		size = std::max(size, term->getSize());
	}
	return size;
}

bool CE::Decompiler::ExprTree::LinearExpr::isFloatingPoint() {
	return IsOperationFloatingPoint(m_operation);
}

INode* CE::Decompiler::ExprTree::LinearExpr::clone(NodeCloneContext* ctx) {
	const auto clonedConstTerm = dynamic_cast<INumberLeaf*>(m_constTerm->clone());
	auto newLinearExpr = new LinearExpr(clonedConstTerm, m_operation);
	for (auto term : m_terms) {
		newLinearExpr->addTerm(term->clone(ctx));
	}
	return newLinearExpr;
}

HS CE::Decompiler::ExprTree::LinearExpr::getHash() {
	HS hs;
	if (IsOperationMoving(m_operation)) {
		for (auto term : m_terms) {
			hs = hs + term->getHash();
		}
	}
	else {
		for (auto term : m_terms) {
			hs = hs << term->getHash();
		}
	}
	return hs
		<< m_constTerm->getHash()
		<< static_cast<int>(m_operation);
}

std::string CE::Decompiler::ExprTree::LinearExpr::printDebug() {
	std::string result = "(";
	for (auto it = m_terms.begin(); it != m_terms.end(); it++) {
		result += (*it)->printDebug();
		if (it != std::prev(m_terms.end()) || m_constTerm->getValue()) {
			result += " " + ShowOperation(m_operation) + OperationalNode::getOpSize(getSize(), isFloatingPoint()) + " ";
		}
	}

	if (m_constTerm->getValue()) {
		result += m_constTerm->printDebug();
	}

	result += ")";
	return (m_updateDebugInfo = result);
}
