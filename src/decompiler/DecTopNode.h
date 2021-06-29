#pragma once
#include "ExprTree/ExprTreeNode.h"

namespace CE::Decompiler
{
	class TopNode : public ExprTree::INodeAgregator
	{
		ExprTree::INode* m_node;
	public:
		TopNode(ExprTree::INode* node) {
			if (node) {
				setNode(node);
			}
		}

		virtual ~TopNode() {
			clear();
		}

		void replaceNode(ExprTree::INode* node, ExprTree::INode* newNode) override {
			if (getNode() == node) {
				m_node = newNode;
			}
		}

		std::list<ExprTree::INode*> getNodesList() override {
			return { getNode() };
		}

		ExprTree::INode* getNode() {
			return m_node;
		}

		void setNode(ExprTree::INode* node) {
			m_node = node;
			node->addParentNode(this);
		}

		void clear() {
			if (m_node) {
				m_node->removeBy(this);
				m_node = nullptr;
			}
		}
	};
};