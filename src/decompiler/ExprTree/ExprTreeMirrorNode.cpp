#include "ExprTreeMirrorNode.h"

using namespace CE::Decompiler;
using namespace CE::Decompiler::ExprTree;

CE::Decompiler::ExprTree::MirrorNode::MirrorNode(INode* node, PCode::Instruction* instr)
	: m_node(node), m_instr(instr)
{
	m_node->addParentNode(this);
}

CE::Decompiler::ExprTree::MirrorNode::~MirrorNode() {
	m_node->removeBy(this);
}

void CE::Decompiler::ExprTree::MirrorNode::replaceNode(INode* node, INode* newNode) {
	if (m_node == node) {
		m_node = newNode;
	}
}

std::list<ExprTree::INode*> CE::Decompiler::ExprTree::MirrorNode::getNodesList() {
	return { m_node };
}

std::list<PCode::Instruction*> CE::Decompiler::ExprTree::MirrorNode::getInstructionsRelatedTo() {
	if (m_instr)
		return { m_instr };
	return {};
}

int CE::Decompiler::ExprTree::MirrorNode::getSize() {
	return m_node->getSize();
}

bool CE::Decompiler::ExprTree::MirrorNode::isFloatingPoint() {
	return m_node->isFloatingPoint();
}

INode* CE::Decompiler::ExprTree::MirrorNode::clone(NodeCloneContext* ctx) {
	return new MirrorNode(m_node->clone(ctx), m_instr);
}

HS CE::Decompiler::ExprTree::MirrorNode::getHash() {
	return m_node->getHash();
}

std::string CE::Decompiler::ExprTree::MirrorNode::printDebug() {
	return m_node->printDebug();
}
