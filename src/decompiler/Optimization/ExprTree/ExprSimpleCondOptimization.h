#pragma once
#include "ExprModification.h"
#include "ExprConstCalculating.h"

namespace CE::Decompiler::Optimization
{
	// Simplify simple condition: SBORROW, rax + -0x2 < 0 -> rax < -0x2 * -1
	class ExprSimpleConditionOptimization : public ExprModification
	{
	public:
		ExprSimpleConditionOptimization(Condition* cond);

		void start() override;

		Condition* getCondition();
	private:
		//replace SBORROW condition with normal
		//SBORROW(*(uint_32t*)([reg_rsp_64]), 0x4{4}) == ((*(uint_32t*)([reg_rsp_64]) + 0x3fffffffc{-4}) < 0x0{0}))
		bool processSBORROW(Condition* cond);

		//rax + -0x2 < 0 -> rax < -0x2 * -1
		//if(((((([mem_2_32] *.4 0x4{4}) >>.4 0x2{2}) *.4 0xffffffff{-1}) +.4 [mem_3_32]) == 0x0{0})) -> if(([mem_3_32] == ((([mem_2_32] *.4 0x4{4}) >>.4 0x2{2}) *.4 0x1{1})))
		bool moveTermToRightPartOfCondition(Condition* cond);

		static void OptimizeNode(INode* node);

		//check negative arithmetic sign of expr node
		static bool IsNegative(INode* node, int size);
	};
};