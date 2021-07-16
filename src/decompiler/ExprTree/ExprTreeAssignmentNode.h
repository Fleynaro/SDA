#pragma once
#include "ExprTreeNode.h"

namespace CE::Decompiler::ExprTree
{
	class AssignmentNode : public Node, public INodeAgregator, public PCode::IRelatedToInstruction
	{
		INode* m_dstNode;
		INode* m_srcNode;
	public:
		PCode::Instruction* m_instr;

		AssignmentNode(INode* dstNode, INode* srcNode, PCode::Instruction* instr = nullptr)
			: m_dstNode(dstNode), m_srcNode(srcNode), m_instr(instr)
		{
			m_dstNode->addParentNode(this);
			m_srcNode->addParentNode(this);
		}

		~AssignmentNode() override;

		void replaceNode(INode* node, INode* newNode) override;

		std::list<INode*> getNodesList() override;

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override;

		INode* getDstNode() const;

		INode* getSrcNode() const;

		void setDstNode(INode* node);

		void setSrcNode(INode* node);

		int getSize() override;

		HS getHash() override;

		INode* clone(NodeCloneContext* ctx) override;
	};
};