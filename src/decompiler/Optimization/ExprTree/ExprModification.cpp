#include "ExprModification.h"

using namespace CE::Decompiler;

// replace this node with another

ExprModification::ExprModification(INode* node)
	: m_node(node)
{
	UpdateDebugInfo(m_node);
}

INode* ExprModification::getNode() const
{
	return m_node;
}

bool ExprModification::isChanged() const
{
	return m_isChanged;
}

void ExprModification::markAsChanged() {
	m_isChanged = true;
}

void ExprModification::replace(INode* newNode, bool destroy) {
	m_node->replaceWith(newNode);
	if (destroy) {
		delete m_node;
	}
	m_node = newNode;
	markAsChanged();
	UpdateDebugInfo(m_node);
}
