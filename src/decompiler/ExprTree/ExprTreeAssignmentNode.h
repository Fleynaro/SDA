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

		~AssignmentNode() {
			if (m_dstNode)
				m_dstNode->removeBy(this);
			if (m_srcNode)
				m_srcNode->removeBy(this);
		}

		void replaceNode(INode* node, INode* newNode) override {
			if (node == m_dstNode) {
				m_dstNode = newNode;
			}
			if (node == m_srcNode) {
				m_srcNode = newNode;
			}
		}

		std::list<INode*> getNodesList() override {
			return { m_dstNode, m_srcNode };
		}

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override {
			if (m_instr)
				return { m_instr };
			//return {};

			// upd: this code down is placed from the class GraphLocalVarsRelToInstructions
			std::list<PCode::Instruction*> list;
			if (auto nodeRelToInstr = dynamic_cast<PCode::IRelatedToInstruction*>(getSrcNode())) {
				auto list2 = nodeRelToInstr->getInstructionsRelatedTo();
				list.insert(list.end(), list2.begin(), list2.end());
			}
			return list;
		}

		INode* getDstNode() {
			return m_dstNode;
		}

		INode* getSrcNode() {
			return m_srcNode;
		}

		void setDstNode(INode* node) {
			m_dstNode->removeBy(this);
			m_dstNode = node;
			node->addParentNode(this);
		}

		void setSrcNode(INode* node) {
			m_srcNode->removeBy(this);
			m_srcNode = node;
			node->addParentNode(this);
		}

		int getSize() override {
			return m_srcNode->getSize();
		}

		HS getHash() override {
			return m_dstNode->getHash() << m_srcNode->getHash();
		}

		INode* clone(NodeCloneContext* ctx) override {
			return new AssignmentNode(m_dstNode->clone(ctx), m_srcNode->clone(ctx));
		}

		std::string printDebug() override {
			return m_dstNode->printDebug() + " = " + m_srcNode->printDebug() + "\n";
		}
	};
};