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
		ExprExpandingToLinearExpr(OperationalNode* node);

		void start() override;

		OperationalNode* getOpNode();
	private:
		// using terms (including constant term) build linear expression
		LinearExpr* buildLinearExpr();

		//(5x - 10y) * 2 + 5 ->	{x: 10, y: -20, constTerm: 5}
		void defineTerms(INode* node, int64_t k, int level = 0);

		// arithmetic/logic/floating operation state
		bool defineOperationState(OperationType op);
	};
};