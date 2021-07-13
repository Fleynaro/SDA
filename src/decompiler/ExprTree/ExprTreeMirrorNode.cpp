#include "ExprTreeMirrorNode.h"

using namespace CE::Decompiler;
using namespace ExprTree;

MirrorNode::MirrorNode(INode* node, PCode::Instruction* instr)
	: m_node(node), m_instr(instr)
{
	m_node->addParentNode(this);
}

MirrorNode::~MirrorNode() {
	m_node->removeBy(this);
}

void MirrorNode::replaceNode(INode* node, INode* newNode) {
	if (m_node == node) {
		m_node = newNode;
	}
}

std::list<INode*> MirrorNode::getNodesList() {
	return { m_node };
}

std::list<PCode::Instruction*> MirrorNode::getInstructionsRelatedTo() {
	if (m_instr)
		return { m_instr };
	return {};
}

int MirrorNode::getSize() {
	return m_node->getSize();
}

bool MirrorNode::isFloatingPoint() {
	return m_node->isFloatingPoint();
}

INode* MirrorNode::clone(NodeCloneContext* ctx) {
	return new MirrorNode(m_node->clone(ctx), m_instr);
}

HS MirrorNode::getHash() {
	return m_node->getHash();
}

std::string MirrorNode::printDebug() {
	return m_node->printDebug();
}
