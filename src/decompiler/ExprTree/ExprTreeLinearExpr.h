#pragma once
#include "ExprTreeOperationalNode.h"

namespace CE::Decompiler::ExprTree
{
	class LinearExpr : public Node, public INodeAgregator, public PCode::IRelatedToInstruction
	{
		std::list<INode*> m_terms;
		INumberLeaf* m_constTerm;
	public:
		OperationType m_operation;
		
		LinearExpr(INumberLeaf* constTerm, OperationType operation = Add)
			: m_constTerm(constTerm), m_operation(operation)
		{
			m_constTerm->addParentNode(this);
		}

		~LinearExpr() {
			for (auto term : m_terms) {
				term->removeBy(this);
			}
			m_constTerm->removeBy(this);
		}

		void addTerm(ExprTree::INode* term) {
			term->addParentNode(this);
			m_terms.push_back(term);
		}

		void setConstTermValue(int64_t constTerm) {
			m_constTerm->setValue((uint64_t)constTerm);
		}

		int64_t getConstTermValue() {
			return (int64_t)m_constTerm->getValue();
		}

		std::list<ExprTree::INode*>& getTerms() {
			return m_terms;
		}

		INumberLeaf* getConstTerm() {
			return m_constTerm;
		}

		void replaceNode(ExprTree::INode* node, ExprTree::INode* newNode) override {
			for (auto it = m_terms.begin(); it != m_terms.end(); it ++) {
				if (node == *it) {
					*it = newNode;
				}
			}
			if (node == m_constTerm)
				m_constTerm = dynamic_cast<INumberLeaf*>(newNode);
		}

		std::list<INode*> getNodesList() override {
			auto list = m_terms;
			list.push_back(m_constTerm);
			return list;
		}

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override {
			return {};
		}

		int getSize() override {
			auto size = 1;
			for (auto term : m_terms) {
				size = max(size, term->getSize());
			}
			return size;
		}

		bool isFloatingPoint() override {
			return IsOperationFloatingPoint(m_operation);
		}

		INode* clone(NodeCloneContext* ctx) override {
			auto clonedConstTerm = dynamic_cast<INumberLeaf*>(m_constTerm->clone());
			auto newLinearExpr = new LinearExpr(clonedConstTerm, m_operation);
			for (auto term : m_terms) {
				newLinearExpr->addTerm(term->clone(ctx));
			}
			return newLinearExpr;
		}

		HS getHash() override {
			HS hs;
			if (IsOperationMoving(m_operation)) {
				for (auto term : m_terms) {
					hs = hs + term->getHash();
				}
			}
			else {
				for (auto term : m_terms) {
					hs = hs << term->getHash();
				}
			}
			return hs
				<< m_constTerm->getHash()
				<< (int)m_operation;
		}

		std::string printDebug() override {
			std::string result = "(";
			for (auto it = m_terms.begin(); it != m_terms.end(); it ++) {
				result += (*it)->printDebug();
				if (it != std::prev(m_terms.end()) || m_constTerm->getValue()) {
					result += " " + ShowOperation(m_operation) + OperationalNode::getOpSize(getSize(), isFloatingPoint()) + " ";
				}
			}

			if (m_constTerm->getValue()) {
				result += m_constTerm->printDebug();
			}

			result += ")";
			return (m_updateDebugInfo = result);
		}
	};
};