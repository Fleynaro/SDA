#include "ExprTreeSdaUnkLocation.h"

using namespace CE;
using namespace CE::Decompiler;
using namespace CE::Decompiler::ExprTree;

CE::Decompiler::ExprTree::UnknownLocation::UnknownLocation(LinearExpr* linearExpr, int baseNodeIdx)
	: m_linearExpr(linearExpr), m_baseNodeIdx(baseNodeIdx)
{}

CE::Decompiler::ExprTree::UnknownLocation::~UnknownLocation() {
	m_linearExpr->removeBy(this);
}

ISdaNode* CE::Decompiler::ExprTree::UnknownLocation::getBaseSdaNode() const
{
	int idx = 0;
	for (auto termNode : m_linearExpr->getTerms()) {
		if (idx++ == m_baseNodeIdx)
			return dynamic_cast<ISdaNode*>(termNode);
	}
	return nullptr;
}

LinearExpr* CE::Decompiler::ExprTree::UnknownLocation::getLinearExpr() const
{
	return m_linearExpr;
}

void CE::Decompiler::ExprTree::UnknownLocation::setConstTermValue(int64_t constTerm) const
{
	m_linearExpr->setConstTermValue(constTerm);
}

int64_t CE::Decompiler::ExprTree::UnknownLocation::getConstTermValue() const
{
	return m_linearExpr->getConstTermValue();
}

std::list<UnknownLocation::Term> CE::Decompiler::ExprTree::UnknownLocation::getArrTerms() const
{
	std::list<Term> terms;
	int idx = 0;
	for (auto termNode : m_linearExpr->getTerms()) {
		if (idx++ == m_baseNodeIdx)
			continue;
		Term term;
		term.m_node = dynamic_cast<ISdaNode*>(termNode);
		terms.push_back(term);
	}
	return terms;
}

void CE::Decompiler::ExprTree::UnknownLocation::replaceNode(ExprTree::INode* node, ExprTree::INode* newNode) {
	if (node == m_linearExpr)
		m_linearExpr = dynamic_cast<LinearExpr*>(newNode);
}

std::list<INode*> CE::Decompiler::ExprTree::UnknownLocation::getNodesList() {
	return m_linearExpr->getNodesList();
}

std::list<PCode::Instruction*> CE::Decompiler::ExprTree::UnknownLocation::getInstructionsRelatedTo() {
	return m_linearExpr->getInstructionsRelatedTo();
}

int CE::Decompiler::ExprTree::UnknownLocation::getSize() {
	return m_linearExpr->getSize();
}

HS CE::Decompiler::ExprTree::UnknownLocation::getHash() {
	return m_linearExpr->getHash(); //todo: + term hashes
}

ISdaNode* CE::Decompiler::ExprTree::UnknownLocation::cloneSdaNode(NodeCloneContext* ctx) {
	auto clonedLinearExpr = dynamic_cast<LinearExpr*>(m_linearExpr->clone(ctx));
	const auto newUnknownLocation = new UnknownLocation(clonedLinearExpr, m_baseNodeIdx);
	clonedLinearExpr->addParentNode(newUnknownLocation);
	return newUnknownLocation;
}

DataTypePtr CE::Decompiler::ExprTree::UnknownLocation::getSrcDataType() {
	return getBaseSdaNode()->getDataType();
}

void CE::Decompiler::ExprTree::UnknownLocation::setDataType(DataTypePtr dataType) {
	getBaseSdaNode()->setDataType(dataType);
}

void CE::Decompiler::ExprTree::UnknownLocation::getLocation(MemLocation& location) {
	auto baseSdaNode = getBaseSdaNode();
	auto valueDataType = CloneUnit(baseSdaNode->getDataType());
	valueDataType->removePointerLevelOutOfFront();

	if (auto locatableNode = dynamic_cast<ILocatable*>(baseSdaNode)) {
		locatableNode->getLocation(location);
	}
	else {
		location.m_type = MemLocation::IMPLICIT;
		location.m_baseAddrHash = baseSdaNode->getHash();
	}
	location.m_offset += getConstTermValue();
	location.m_valueSize = valueDataType->getSize();
	for (auto term : getArrTerms()) {
		auto multiplier = term.getMultiplier();
		const auto itemSize = multiplier ? static_cast<int>(multiplier->getValue()) : 1;
		location.addArrayDim(itemSize);
	}
}

std::string CE::Decompiler::ExprTree::UnknownLocation::printSdaDebug() {
	return m_linearExpr->printDebug();
}
