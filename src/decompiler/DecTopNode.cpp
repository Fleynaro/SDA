#include "DecTopNode.h"

using namespace CE::Decompiler;

CE::Decompiler::TopNode::~TopNode() {
	clear();
}

void CE::Decompiler::TopNode::replaceNode(ExprTree::INode* node, ExprTree::INode* newNode) {
	if (getNode() == node) {
		m_node = newNode;
	}
}

std::list<ExprTree::INode*> CE::Decompiler::TopNode::getNodesList() {
	return { getNode() };
}

ExprTree::INode* CE::Decompiler::TopNode::getNode() {
	return m_node;
}

void CE::Decompiler::TopNode::setNode(ExprTree::INode* node) {
	m_node = node;
	node->addParentNode(this);
}

void CE::Decompiler::TopNode::clear() {
	if (m_node) {
		m_node->removeBy(this);
		m_node = nullptr;
	}
}
