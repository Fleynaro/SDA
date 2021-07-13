#include "DecTopNode.h"

using namespace CE::Decompiler;

TopNode::TopNode(ExprTree::INode* node)
{
	if (node)
	{
		setNode(node);
	}
}

TopNode::~TopNode() {
	clear();
}

void TopNode::replaceNode(ExprTree::INode* node, ExprTree::INode* newNode) {
	if (getNode() == node) {
		m_node = newNode;
	}
}

std::list<ExprTree::INode*> TopNode::getNodesList() {
	return { getNode() };
}

ExprTree::INode* TopNode::getNode() const
{
	return m_node;
}

void TopNode::setNode(ExprTree::INode* node) {
	m_node = node;
	node->addParentNode(this);
}

void TopNode::clear() {
	if (m_node) {
		m_node->removeBy(this);
		m_node = nullptr;
	}
}
