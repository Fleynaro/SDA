#include "ExprModification.h"

using namespace CE::Decompiler;

// replace this node with another

CE::Decompiler::ExprModification::ExprModification(INode* node)
	: m_node(node)
{
	INode::UpdateDebugInfo(m_node);
}

INode* CE::Decompiler::ExprModification::getNode() const
{
	return m_node;
}

bool CE::Decompiler::ExprModification::isChanged() const
{
	return m_isChanged;
}

void CE::Decompiler::ExprModification::changed() {
	m_isChanged = true;
}

void CE::Decompiler::ExprModification::replace(INode* newNode, bool destroy) {
	m_node->replaceWith(newNode);
	if (destroy) {
		delete m_node;
	}
	m_node = newNode;
	changed();
	INode::UpdateDebugInfo(m_node);
}
