#include "ExprTreeSdaGenericNode.h"

using namespace CE;
using namespace Decompiler;
using namespace ExprTree;

SdaGenericNode::SdaGenericNode(INode* node, DataTypePtr calcDataType)
	: m_node(node), m_calcDataType(calcDataType)
{}

SdaGenericNode::~SdaGenericNode() {
	m_node->removeBy(this);
}

void SdaGenericNode::replaceNode(INode* node, INode* newNode) {
	if (m_node == node) {
		m_node = newNode;
	}
}

std::list<INode*> SdaGenericNode::getNodesList() {
	return { m_node };
}

INode* SdaGenericNode::getNode() const
{
	return m_node;
}

DataTypePtr SdaGenericNode::getSrcDataType() {
	return m_calcDataType;
}

void SdaGenericNode::setDataType(DataTypePtr dataType) {
	m_calcDataType = dataType;
}

int SdaGenericNode::getSize() {
	return m_node->getSize();
}

bool SdaGenericNode::isFloatingPoint() {
	return m_node->isFloatingPoint();
}

HS SdaGenericNode::getHash() {
	return m_node->getHash();
}

ISdaNode* SdaGenericNode::cloneSdaNode(NodeCloneContext* ctx) {
	auto clonedNode = m_node->clone(ctx);
	const auto sdaNode = new SdaGenericNode(clonedNode, CloneUnit(m_calcDataType));
	clonedNode->addParentNode(sdaNode);
	return sdaNode;
}

std::string SdaGenericNode::printSdaDebug() {
	auto result = m_node->printDebug();
	if (const auto readValueNode = dynamic_cast<ReadValueNode*>(m_node))
		result = "*" + readValueNode->getAddress()->printDebug();
	return result;
}
