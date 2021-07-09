#include "ExprTreeSdaGenericNode.h"

using namespace CE;
using namespace CE::Decompiler;
using namespace CE::Decompiler::ExprTree;

CE::Decompiler::ExprTree::SdaGenericNode::SdaGenericNode(INode* node, DataTypePtr calcDataType)
	: m_node(node), m_calcDataType(calcDataType)
{}

CE::Decompiler::ExprTree::SdaGenericNode::~SdaGenericNode() {
	m_node->removeBy(this);
}

void CE::Decompiler::ExprTree::SdaGenericNode::replaceNode(INode* node, INode* newNode) {
	if (m_node == node) {
		m_node = newNode;
	}
}

std::list<ExprTree::INode*> CE::Decompiler::ExprTree::SdaGenericNode::getNodesList() {
	return { m_node };
}

INode* CE::Decompiler::ExprTree::SdaGenericNode::getNode() const
{
	return m_node;
}

DataTypePtr CE::Decompiler::ExprTree::SdaGenericNode::getSrcDataType() {
	return m_calcDataType;
}

void CE::Decompiler::ExprTree::SdaGenericNode::setDataType(DataTypePtr dataType) {
	m_calcDataType = dataType;
}

int CE::Decompiler::ExprTree::SdaGenericNode::getSize() {
	return m_node->getSize();
}

bool CE::Decompiler::ExprTree::SdaGenericNode::isFloatingPoint() {
	return m_node->isFloatingPoint();
}

HS CE::Decompiler::ExprTree::SdaGenericNode::getHash() {
	return m_node->getHash();
}

ISdaNode* CE::Decompiler::ExprTree::SdaGenericNode::cloneSdaNode(NodeCloneContext* ctx) {
	auto clonedNode = m_node->clone(ctx);
	const auto sdaNode = new SdaGenericNode(clonedNode, CloneUnit(m_calcDataType));
	clonedNode->addParentNode(sdaNode);
	return sdaNode;
}

std::string CE::Decompiler::ExprTree::SdaGenericNode::printSdaDebug() {
	auto result = m_node->printDebug();
	if (const auto readValueNode = dynamic_cast<ReadValueNode*>(m_node))
		result = "*" + readValueNode->getAddress()->printDebug();
	return result;
}
