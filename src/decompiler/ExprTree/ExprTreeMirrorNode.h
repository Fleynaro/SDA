#pragma once
#include "ExprTreeNode.h"

namespace CE::Decompiler::ExprTree
{
	class MirrorNode : public Node, public INodeAgregator, public PCode::IRelatedToInstruction
	{
	public:
		INode* m_node;
		PCode::Instruction* m_instr;

		MirrorNode(INode* node, PCode::Instruction* instr);

		~MirrorNode();

		void replaceNode(INode* node, INode* newNode) override;

		std::list<ExprTree::INode*> getNodesList() override;

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override;

		int getSize() override;

		bool isFloatingPoint() override;

		INode* clone(NodeCloneContext* ctx) override;

		HS getHash() override;

		std::string printDebug() override;
	};
};