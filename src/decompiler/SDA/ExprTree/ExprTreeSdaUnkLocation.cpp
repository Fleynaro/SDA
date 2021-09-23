#include "ExprTreeSdaUnkLocation.h"

using namespace CE;
using namespace Decompiler;
using namespace ExprTree;

UnknownLocation::UnknownLocation(LinearExpr* linearExpr, int baseNodeIdx)
	: m_linearExpr(linearExpr), m_baseNodeIdx(baseNodeIdx)
{}

UnknownLocation::~UnknownLocation() {
	m_linearExpr->removeBy(this);
}

ISdaNode* UnknownLocation::getBaseSdaNode() const
{
	int idx = 0;
	for (auto termNode : m_linearExpr->getTerms()) {
		if (idx++ == m_baseNodeIdx)
			return dynamic_cast<ISdaNode*>(termNode);
	}
	return nullptr;
}

LinearExpr* UnknownLocation::getLinearExpr() const
{
	return m_linearExpr;
}

void UnknownLocation::setConstTermValue(int64_t constTerm) const
{
	m_linearExpr->setConstTermValue(constTerm);
}

int64_t UnknownLocation::getConstTermValue() const
{
	return m_linearExpr->getConstTermValue();
}

std::list<UnknownLocation::Term> UnknownLocation::getArrTerms() const
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

void UnknownLocation::replaceNode(INode* node, INode* newNode) {
	if (node == m_linearExpr)
		m_linearExpr = dynamic_cast<LinearExpr*>(newNode);
}

std::list<INode*> UnknownLocation::getNodesList() {
	return m_linearExpr->getNodesList();
}

std::list<PCode::Instruction*> UnknownLocation::getInstructionsRelatedTo() {
	return m_linearExpr->getInstructionsRelatedTo();
}

int UnknownLocation::getSize() {
	return m_linearExpr->getSize();
}

HS UnknownLocation::getHash() {
	return m_linearExpr->getHash(); //todo: + term hashes
}

ISdaNode* UnknownLocation::cloneSdaNode(NodeCloneContext* ctx) {
	auto clonedLinearExpr = dynamic_cast<LinearExpr*>(m_linearExpr->clone(ctx));
	const auto newUnknownLocation = new UnknownLocation(clonedLinearExpr, m_baseNodeIdx);
	clonedLinearExpr->addParentNode(newUnknownLocation);
	return newUnknownLocation;
}

DataTypePtr UnknownLocation::getSrcDataType() {
	return getBaseSdaNode()->getDataType();
}

void UnknownLocation::setDataType(DataTypePtr dataType) {
	getBaseSdaNode()->setDataType(dataType);
}

bool UnknownLocation::getLocation(MemLocation& location) {
	const auto baseSdaNode = getBaseSdaNode();
	if (const auto locatableNode = dynamic_cast<ILocatable*>(baseSdaNode)) {
		if (!locatableNode->getLocation(location))
			return false;
	}
	else {
		location.m_type = MemLocation::IMPLICIT;
		location.m_baseAddrHash = baseSdaNode->getHash();
	}
	location.m_offset += getConstTermValue();

	// get item size of array
	const auto valueDataType = CloneUnit(baseSdaNode->getSrcDataType());
	valueDataType->removePointerLevelFromTop();
	location.m_valueSize = valueDataType->getSize();
	
	for (const auto term : getArrTerms()) {
		auto multiplier = term.getMultiplier();
		const auto itemSize = multiplier ? static_cast<int>(multiplier->getValue()) : 1;
		location.addArrayDim(itemSize);
	}
	return true;
}