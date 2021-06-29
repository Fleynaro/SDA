#pragma once
#include "ExprTreeNode.h"

namespace CE::Decompiler::ExprTree
{
	class MirrorNode : public Node, public INodeAgregator, public PCode::IRelatedToInstruction
	{
	public:
		INode* m_node;
		PCode::Instruction* m_instr;

		MirrorNode(INode* node, PCode::Instruction* instr)
			: m_node(node), m_instr(instr)
		{
			m_node->addParentNode(this);
		}

		~MirrorNode() {
			m_node->removeBy(this);
		}

		void replaceNode(INode* node, INode* newNode) override {
			if (m_node == node) {
				m_node = newNode;
			}
		}

		std::list<ExprTree::INode*> getNodesList() override {
			return { m_node };
		}

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override {
			if (m_instr)
				return { m_instr };
			return {};
		}

		int getSize() override {
			return m_node->getSize();
		}

		bool isFloatingPoint() override {
			return m_node->isFloatingPoint();
		}

		INode* clone(NodeCloneContext* ctx) override {
			return new MirrorNode(m_node->clone(ctx), m_instr);
		}

		HS getHash() override {
			return m_node->getHash();
		}

		std::string printDebug() override {
			return m_node->printDebug();
		}
	};
};