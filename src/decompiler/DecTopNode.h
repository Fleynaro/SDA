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

		virtual ~TopNode();

		void replaceNode(ExprTree::INode* node, ExprTree::INode* newNode) override;

		std::list<ExprTree::INode*> getNodesList() override;

		ExprTree::INode* getNode();

		void setNode(ExprTree::INode* node);

		void clear();
	};
};