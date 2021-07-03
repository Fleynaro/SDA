#include "ExprTreeNode.h"

using namespace CE::Decompiler::ExprTree;

// replace this node with another, remove all associations and make this node independent from expression tree

void CE::Decompiler::ExprTree::Node::replaceWith(INode* newNode) {
	auto it = m_parentNodes.begin();
	while(it != m_parentNodes.end())
	{
		auto parentNode = *it;
		if (newNode == dynamic_cast<INode*>(parentNode))
			continue;
		parentNode->replaceNode(this, newNode);
		if (newNode != nullptr) {
			newNode->addParentNode(parentNode);
		}
		it = m_parentNodes.erase(it);
	}
}

void CE::Decompiler::ExprTree::Node::removeBy(INodeAgregator* node) {
	if (node != nullptr) {
		node->replaceNode(this, nullptr);
		removeParentNode(node);
	}
	if (m_parentNodes.size() == 0)
		delete this;
}

void CE::Decompiler::ExprTree::Node::addParentNode(INodeAgregator* node) {
	if (this == dynamic_cast<INode*>(node))
		return;
	m_parentNodes.push_back(node);
}

void CE::Decompiler::ExprTree::Node::removeParentNode(INodeAgregator* node) {
	m_parentNodes.remove(node);
}

std::list<INodeAgregator*>& CE::Decompiler::ExprTree::Node::getParentNodes() {
	return m_parentNodes;
}

// get single parent (exception thrown because of multiple parents)

INodeAgregator* CE::Decompiler::ExprTree::Node::getParentNode() {
	if (m_parentNodes.size() != 1) {
		throw std::logic_error("it is ambigious because of multiple parents");
	}
	return *m_parentNodes.begin();
}


// not integer type

bool CE::Decompiler::ExprTree::Node::isFloatingPoint() {
	return false;
}

std::string CE::Decompiler::ExprTree::Node::printDebug() {
	return "";
}

INode* CE::Decompiler::ExprTree::INode::clone() {
	NodeCloneContext ctx;
	return clone(&ctx);
}

void CE::Decompiler::ExprTree::INode::iterateChildNodes(std::function<void(INode*)> func) {
	if (auto agregator = dynamic_cast<INodeAgregator*>(this)) {
		auto list = agregator->getNodesList();
		for (auto node : list) {
			if (node) {
				func(node);
			}
		}
	}
}

void CE::Decompiler::ExprTree::INode::UpdateDebugInfo(INode* node) {
	if (!node) return;
	node->printDebug();
	//node->checkOnSingleParents();
}

void CE::Decompiler::ExprTree::INode::checkOnSingleParents() {
	auto parentNode = getParentNode();
	if (auto nodeAgregator = dynamic_cast<INodeAgregator*>(this)) {
		for (auto childNode : nodeAgregator->getNodesList())
			if (childNode)
				childNode->checkOnSingleParents();
	}
}
