#include "ExprTreeAssignmentNode.h"

using namespace CE::Decompiler;
using namespace ExprTree;

AssignmentNode::~AssignmentNode() {
	if (m_dstNode)
		m_dstNode->removeBy(this);
	if (m_srcNode)
		m_srcNode->removeBy(this);
}

void AssignmentNode::replaceNode(INode* node, INode* newNode) {
	if (node == m_dstNode) {
		m_dstNode = newNode;
	}
	if (node == m_srcNode) {
		m_srcNode = newNode;
	}
}

std::list<INode*> AssignmentNode::getNodesList() {
	return { m_dstNode, m_srcNode };
}

std::list<PCode::Instruction*> AssignmentNode::getInstructionsRelatedTo() {
	if (m_instr)
		return { m_instr };
	//return {};

	// upd: this code down is placed from the class GraphLocalVarsRelToInstructions
	std::list<PCode::Instruction*> list;
	if (auto nodeRelToInstr = dynamic_cast<IRelatedToInstruction*>(getSrcNode())) {
		auto list2 = nodeRelToInstr->getInstructionsRelatedTo();
		list.insert(list.end(), list2.begin(), list2.end());
	}
	return list;
}

INode* AssignmentNode::getDstNode() const
{
	return m_dstNode;
}

INode* AssignmentNode::getSrcNode() const
{
	return m_srcNode;
}

void AssignmentNode::setDstNode(INode* node) {
	m_dstNode->removeBy(this);
	m_dstNode = node;
	node->addParentNode(this);
}

void AssignmentNode::setSrcNode(INode* node) {
	m_srcNode->removeBy(this);
	m_srcNode = node;
	node->addParentNode(this);
}

int AssignmentNode::getSize() {
	return m_srcNode->getSize();
}

HS AssignmentNode::getHash() {
	return m_dstNode->getHash() << m_srcNode->getHash();
}

INode* AssignmentNode::clone(NodeCloneContext* ctx) {
	return new AssignmentNode(m_dstNode->clone(ctx), m_srcNode->clone(ctx));
}
