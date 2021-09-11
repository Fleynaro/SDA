#include "ExprTreeNode.h"
#include "decompiler/DecCodeGenerator.h"

using namespace CE::Decompiler::ExprTree;

// replace this node with another, remove all associations and make this node independent from expression tree

void Node::replaceWith(INode* newNode) {
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

void Node::removeBy(INodeAgregator* node) {
	if (node != nullptr) {
		node->replaceNode(this, nullptr);
		removeParentNode(node);
	}
	if (m_parentNodes.size() == 0)
		delete this;
}

void Node::addParentNode(INodeAgregator* node) {
	if (this == dynamic_cast<INode*>(node))
		return;
	m_parentNodes.push_back(node);
}

void Node::removeParentNode(INodeAgregator* node) {
	m_parentNodes.remove(node);
}

std::list<INodeAgregator*>& Node::getParentNodes() {
	return m_parentNodes;
}

// get single parent (exception thrown because of multiple parents)

INodeAgregator* Node::getParentNode() {
	if (m_parentNodes.size() != 1) {
		throw std::logic_error("it is ambigious because of multiple parents");
	}
	return *m_parentNodes.begin();
}


// not integer type

bool Node::isFloatingPoint() {
	return false;
}

void CE::Decompiler::ExprTree::UpdateDebugInfo(INode* node) {
	if (!node) return;
	auto gen = ExprTreeTextGenerator();
	gen.m_debugMode = true;
	gen.generateNode(node);
	if(auto node_ = dynamic_cast<Node*>(node))
		node_->m_updateDebugInfo = gen.m_text;
}

INode* INode::clone() {
	NodeCloneContext ctx;
	return clone(&ctx);
}

void INode::iterateChildNodes(std::function<void(INode*)> func) {
	if (const auto agregator = dynamic_cast<INodeAgregator*>(this)) {
		const auto list = agregator->getNodesList();
		for (const auto node : list) {
			if (node) {
				func(node);
			}
		}
	}
}

void INode::checkOnSingleParents() {
	auto parentNode = getParentNode();
	if (auto nodeAgregator = dynamic_cast<INodeAgregator*>(this)) {
		for (auto childNode : nodeAgregator->getNodesList())
			if (childNode)
				childNode->checkOnSingleParents();
	}
}
