#include "ExprTreeSdaReadValue.h"
#include "ExprTreeSdaLeaf.h"
#include "ExprTreeSdaGenericNode.h"

using namespace CE;
using namespace CE::Decompiler;
using namespace CE::Decompiler::ExprTree;

CE::Decompiler::ExprTree::SdaReadValueNode::SdaReadValueNode(ReadValueNode* readValueNode, DataTypePtr outDataType)
	: m_readValueNode(readValueNode), m_outDataType(outDataType)
{}

CE::Decompiler::ExprTree::SdaReadValueNode::~SdaReadValueNode() {
	m_readValueNode->removeBy(this);
}

ISdaNode* CE::Decompiler::ExprTree::SdaReadValueNode::getAddress() const
{
	return dynamic_cast<ISdaNode*>(m_readValueNode->getAddress());
}

void CE::Decompiler::ExprTree::SdaReadValueNode::replaceNode(ExprTree::INode* node, ExprTree::INode* newNode) {
	if (node == m_readValueNode)
		m_readValueNode = dynamic_cast<ReadValueNode*>(newNode);
}

std::list<INode*> CE::Decompiler::ExprTree::SdaReadValueNode::getNodesList() {
	return m_readValueNode->getNodesList();
}

std::list<PCode::Instruction*> CE::Decompiler::ExprTree::SdaReadValueNode::getInstructionsRelatedTo() {
	return m_readValueNode->getInstructionsRelatedTo();
}

int CE::Decompiler::ExprTree::SdaReadValueNode::getSize() {
	return m_readValueNode->getSize();
}

HS CE::Decompiler::ExprTree::SdaReadValueNode::getHash() {
	return m_readValueNode->getHash(); //todo: + term hashes
}

ISdaNode* CE::Decompiler::ExprTree::SdaReadValueNode::cloneSdaNode(NodeCloneContext* ctx) {
	auto clonedReadValueNode = dynamic_cast<ReadValueNode*>(m_readValueNode->clone(ctx));
	const auto sdaReadValueNode = new SdaReadValueNode(clonedReadValueNode, CloneUnit(m_outDataType));
	clonedReadValueNode->addParentNode(sdaReadValueNode);
	return sdaReadValueNode;
}

DataTypePtr CE::Decompiler::ExprTree::SdaReadValueNode::getSrcDataType() {
	return m_outDataType;
}

void CE::Decompiler::ExprTree::SdaReadValueNode::setDataType(DataTypePtr dataType) {
	m_outDataType = dataType;
}

bool CE::Decompiler::ExprTree::SdaReadValueNode::isAddrGetting() {
	return false;
}

void CE::Decompiler::ExprTree::SdaReadValueNode::setAddrGetting(bool toggle) {
}

void CE::Decompiler::ExprTree::SdaReadValueNode::getLocation(MemLocation& location) {
	if (auto locatableAddrNode = dynamic_cast<ILocatable*>(getAddress())) {
		locatableAddrNode->getLocation(location);
		location.m_valueSize = getSize();
		return;
	}
	else {
		ISdaNode* sdaAddrNode = nullptr;
		int64_t offset = 0x0;
		if (const auto symbolLeaf = dynamic_cast<SdaSymbolLeaf*>(getAddress())) {
			sdaAddrNode = symbolLeaf;
		}
		else if (const auto sdaGenNode = dynamic_cast<SdaGenericNode*>(getAddress())) {
			if (auto linearExpr = dynamic_cast<LinearExpr*>(sdaGenNode->getNode())) {
				if (linearExpr->getTerms().size() == 1) {
					if (sdaAddrNode = dynamic_cast<ISdaNode*>(*linearExpr->getTerms().begin())) {
						offset = linearExpr->getConstTermValue();
					}
				}
			}
		}

		if (sdaAddrNode && sdaAddrNode->getSrcDataType()->getSize() == 0x8) {
			location.m_type = MemLocation::IMPLICIT;
			location.m_baseAddrHash = sdaAddrNode->getHash();
			location.m_offset = offset;
			location.m_valueSize = getSize();
			return;
		}
	}
	throw std::exception("impossible to determine the location");
}

std::string CE::Decompiler::ExprTree::SdaReadValueNode::printSdaDebug() {
	auto result = "*" + getAddress()->printDebug();
	return result;
}
