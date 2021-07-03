#pragma once
#include "ExprTree/ExprTreeNode.h"

namespace CE::Decompiler
{
	class TopNode : public ExprTree::INodeAgregator
	{
		ExprTree::INode* m_node = nullptr;
	public:
		TopNode(ExprTree::INode* node);

		virtual ~TopNode();

		void replaceNode(ExprTree::INode* node, ExprTree::INode* newNode) override;

		std::list<ExprTree::INode*> getNodesList() override;

		ExprTree::INode* getNode() const;

		void setNode(ExprTree::INode* node);

		void clear();
	};
};