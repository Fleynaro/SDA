#pragma once
#include "../../ExprTree/ExprTree.h"

namespace CE::Decompiler
{
	using namespace ExprTree;

	// abstract class for some expr modification/simplification
	class ExprModification
	{
		INode* m_node;
	public:
		ExprModification(INode* node)
			: m_node(node)
		{
			INode::UpdateDebugInfo(m_node);
		}

		virtual void start() = 0;

		INode* getNode() {
			return m_node;
		}

		bool isChanged() {
			return m_isChanged;
		}

	protected:
		bool m_isChanged = false;

		void changed() {
			m_isChanged = true;
		}

		// replace this node with another
		void replace(INode* newNode, bool destroy = true) {
			m_node->replaceWith(newNode);
			if (destroy) {
				delete m_node;
			}
			m_node = newNode;
			changed();
			INode::UpdateDebugInfo(m_node);
		}
	};
};