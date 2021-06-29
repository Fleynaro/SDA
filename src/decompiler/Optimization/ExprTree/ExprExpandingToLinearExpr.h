#pragma once
#include "ExprConstCalculating.h"

namespace CE::Decompiler::Optimization
{
	////((y + 3x) + x) * 2 + 5 => (y + 8x) + 5
	//([reg_rbx_64] & 0xffffffff00000000{0} | [var_2_32]) & 0x1f{31} =>	[var_2_32] & 0x1f{31}
	//(x * 2) * 3 => x * 6
	class ExprExpandingToLinearExpr : public ExprModification
	{
		// operation state (set by method defineOperationState)
		OperationType m_operationAdd = None;
		OperationType m_operationMul = None;
		uint64_t m_invisibleMultiplier;

		// terms
		std::map<HS::Value, std::pair<INode*, int64_t>> m_terms;
		int64_t m_constTerm;
		int m_constTermSize = 1;

		// is there a sense to build LinearExpr? (yes for {(5x + 2) * 2}, but no for {5x + 2})
		bool m_doBuilding = false;
	public:
		ExprExpandingToLinearExpr(OperationalNode* node)
			: ExprModification(node)
		{}

		void start() override {
			if (!defineOperationState(getOpNode()->m_operation))
				return;
			// find all terms and know if m_doBuilding is true
			defineTerms(getOpNode(), m_invisibleMultiplier);
			if (m_doBuilding) {
				auto linearExpr = buildLinearExpr();
				replace(linearExpr);
				/*if (m_terms.size() == 1 && m_constTerm == 0x0) {
					auto baseTerm = m_terms.begin()->second.first;
					if (baseTerm != getNode())
						replace(baseTerm);
				}*/
			}
		}

		OperationalNode* getOpNode() {
			return dynamic_cast<OperationalNode*>(getNode());
		}
	private:
		// using terms (including constant term) build linear expression
		LinearExpr* buildLinearExpr() {
			auto constTerm = new NumberLeaf((uint64_t&)m_constTerm, m_constTermSize);
			auto linearExpr = new LinearExpr(constTerm, m_operationAdd); // todo: change size for number
			// iterate over all terms
			for (auto termInfo : m_terms) {
				auto node = termInfo.second.first;
				auto mask = CalculateMask(node);
				auto multiplier = (uint64_t&)termInfo.second.second;
				
				INode* term;
				if ((multiplier & mask.getValue()) == (m_invisibleMultiplier & mask.getValue())) {
					term = termInfo.second.first;
				}
				else {
					auto multiplierLeaf = new NumberLeaf(multiplier, node->getSize());
					term = new OperationalNode(node, multiplierLeaf, m_operationMul);
				}
				linearExpr->addTerm(term);
			}
			return linearExpr;
		}

		//(5x - 10y) * 2 + 5 ->	{x: 10, y: -20, constTerm: 5}
		void defineTerms(INode* node, int64_t k, int level = 0) {
			auto size = node->getSize();

			// called for add operation (e.g. x + 5)
			if (auto numberLeaf = dynamic_cast<INumberLeaf*>(node)) {
				auto constTerm = ExprConstCalculating::Calculate(
					numberLeaf->getValue(),
					k,
					m_operationMul,
					size
				);

				m_constTerm = ExprConstCalculating::Calculate(
					m_constTerm,
					constTerm,
					m_operationAdd,
					size
				);
				m_constTermSize = max(m_constTermSize, size);
				m_doBuilding = true;
				return;
			}

			// if the source expression is enough long, in other words, can be presented as linear expr.
			if (level == 2) {
				m_doBuilding = true;
			}

			//(x * 2) * 3 => x * 6
			if (auto opNode = dynamic_cast<OperationalNode*>(node)) {
				if (opNode->m_operation == m_operationAdd) {
					defineTerms(opNode->m_leftNode, k, level + 1);
					defineTerms(opNode->m_rightNode, k, level + 1);
					return;
				}
				else if (opNode->m_operation == m_operationMul) {
					if (auto rightNumberLeaf = dynamic_cast<INumberLeaf*>(opNode->m_rightNode)) {
						auto newK = ExprConstCalculating::Calculate( // e.g. 5x * 5 -> 25x
							rightNumberLeaf->getValue(),
							k,
							m_operationMul,
							size
						);
						defineTerms(opNode->m_leftNode, newK, level + 1);
						return;
					}
				}
			}

			// if {node} is not OperationalNode then it is a term
			auto hashVal = node->getHash().getHashValue();
			if (m_terms.find(hashVal) == m_terms.end()) {
				m_terms[hashVal] = std::make_pair(node, 0);
			}
			auto newK = ExprConstCalculating::Calculate(
				m_terms[hashVal].second,
				k,
				m_operationAdd,
				size
			);
			m_terms[hashVal] = std::make_pair(node, newK);
		}

		// arithmetic/logic/floating operation state
		bool defineOperationState(OperationType op) {
			if (op == Add || op == Mul) {
				m_operationAdd = Add;
				m_operationMul = Mul;
				m_invisibleMultiplier = 1;
				return true;
			}
			if (op == Or || op == And) {
				m_operationAdd = Or;
				m_operationMul = And;
				m_invisibleMultiplier = (int64_t)-1;
				return true;
			}

			//TODO: for float and double
			//if (op == fAdd || op == fMul) {
			//	m_operationAdd = fAdd;
			//	m_operationMul = fMul;
			//	m_invisibleMultiplier = 0;
			//	return true;
			//}
			return false;
		}
	};
};