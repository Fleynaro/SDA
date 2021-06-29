#pragma once
#include "ExprTreeOperationalNode.h"

namespace CE::Decompiler::ExprTree
{
	class FunctionCall : public Node, public INodeAgregator, public PCode::IRelatedToInstruction
	{
	public:
		INode* m_destination;
		std::vector<INode*> m_paramNodes;
		PCode::Instruction* m_instr;
		Symbol::FunctionResultVar* m_functionResultVar = nullptr;
		
		FunctionCall(INode* destination, PCode::Instruction* instr)
			: m_destination(destination), m_instr(instr)
		{
			m_destination->addParentNode(this);
		}

		~FunctionCall() {
			if (m_destination)
				m_destination->removeBy(this);
			for (auto paramNode : m_paramNodes) {
				paramNode->removeBy(this);
			}
		}

		void replaceNode(INode* node, INode* newNode) override {
			if (m_destination == node) {
				m_destination = newNode;
			}
			else {
				for (auto it = m_paramNodes.begin(); it != m_paramNodes.end(); it ++) {
					if (node == *it) {
						*it = newNode;
					}
				}
			}
		}

		std::list<ExprTree::INode*> getNodesList() override {
			std::list<ExprTree::INode*> list = { m_destination };
			for (auto paramNode : m_paramNodes) {
				list.push_back(paramNode);
			}
			return list;
		}

		INode* getDestination() {
			return m_destination;
		}

		std::vector<INode*>& getParamNodes() {
			return m_paramNodes;
		}

		void addParamNode(INode* node) {
			node->addParentNode(this);
			m_paramNodes.push_back(node);
		}

		int getSize() override {
			return m_functionResultVar ? m_functionResultVar->getSize() : 0x0;
		}

		bool isFloatingPoint() override {
			return false;
		}

		HS getHash() override {
			return m_functionResultVar ? m_functionResultVar->getHash() : m_destination->getHash();
		}

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override {
			if (m_instr)
				return { m_instr };
			return {};
		}

		INode* clone(NodeCloneContext* ctx) override {
			auto funcVar = m_functionResultVar ? dynamic_cast<Symbol::FunctionResultVar*>(m_functionResultVar->clone(ctx)) : nullptr;
			auto funcCallCtx = new FunctionCall(m_destination->clone(ctx), m_instr);
			funcCallCtx->m_functionResultVar = funcVar;
			for (auto paramNode : m_paramNodes) {
				funcCallCtx->addParamNode(paramNode->clone(ctx));
			}
			return funcCallCtx;
		}

		std::string printDebug() override {
			std::string str = "(" + getDestination()->printDebug() + ")(";
			for (auto paramNode : m_paramNodes) {
				str += paramNode->printDebug() + ", ";
			}
			if (!m_paramNodes.empty()) {
				str.pop_back();
				str.pop_back();
			}
			return (m_updateDebugInfo = (str + ")"));
		}
	};
};