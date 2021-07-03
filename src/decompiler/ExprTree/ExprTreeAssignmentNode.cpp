#include "ExprTreeAssignmentNode.h"

using namespace CE::Decompiler;
using namespace CE::Decompiler::ExprTree;

CE::Decompiler::ExprTree::AssignmentNode::~AssignmentNode() {
	if (m_dstNode)
		m_dstNode->removeBy(this);
	if (m_srcNode)
		m_srcNode->removeBy(this);
}

void CE::Decompiler::ExprTree::AssignmentNode::replaceNode(INode* node, INode* newNode) {
	if (node == m_dstNode) {
		m_dstNode = newNode;
	}
	if (node == m_srcNode) {
		m_srcNode = newNode;
	}
}

std::list<INode*> CE::Decompiler::ExprTree::AssignmentNode::getNodesList() {
	return { m_dstNode, m_srcNode };
}

std::list<PCode::Instruction*> CE::Decompiler::ExprTree::AssignmentNode::getInstructionsRelatedTo() {
	if (m_instr)
		return { m_instr };
	//return {};

	// upd: this code down is placed from the class GraphLocalVarsRelToInstructions
	std::list<PCode::Instruction*> list;
	if (auto nodeRelToInstr = dynamic_cast<PCode::IRelatedToInstruction*>(getSrcNode())) {
		auto list2 = nodeRelToInstr->getInstructionsRelatedTo();
		list.insert(list.end(), list2.begin(), list2.end());
	}
	return list;
}

INode* CE::Decompiler::ExprTree::AssignmentNode::getDstNode() const
{
	return m_dstNode;
}

INode* CE::Decompiler::ExprTree::AssignmentNode::getSrcNode() const
{
	return m_srcNode;
}

void CE::Decompiler::ExprTree::AssignmentNode::setDstNode(INode* node) {
	m_dstNode->removeBy(this);
	m_dstNode = node;
	node->addParentNode(this);
}

void CE::Decompiler::ExprTree::AssignmentNode::setSrcNode(INode* node) {
	m_srcNode->removeBy(this);
	m_srcNode = node;
	node->addParentNode(this);
}

int CE::Decompiler::ExprTree::AssignmentNode::getSize() {
	return m_srcNode->getSize();
}

HS CE::Decompiler::ExprTree::AssignmentNode::getHash() {
	return m_dstNode->getHash() << m_srcNode->getHash();
}

INode* CE::Decompiler::ExprTree::AssignmentNode::clone(NodeCloneContext* ctx) {
	return new AssignmentNode(m_dstNode->clone(ctx), m_srcNode->clone(ctx));
}

std::string CE::Decompiler::ExprTree::AssignmentNode::printDebug() {
	return m_dstNode->printDebug() + " = " + m_srcNode->printDebug() + "\n";
}
