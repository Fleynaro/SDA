#pragma once
#include "ExprModification.h"

namespace CE::Decompiler::Optimization
{
	// Make arithmetic/logic simplification: 5 + 2 => 7, x * 0 -> 0, x & x => x
	class ExprConstCalculating : public ExprModification
	{
	public:
		ExprConstCalculating(OperationalNode* opNode);

		// make calculation of two constant operands ({size} need for float/double operations)
		static uint64_t Calculate(uint64_t op1, uint64_t op2, OperationType operation, int size = 0x8);

		void start() override;

		OperationalNode* getOpNode();
	private:
		//5 + 2 => 7
		bool processConstOperands(OperationalNode* opNode);

		//[var_2_32] * 0							=>		0
		//[var_2_32] ^ [var_2_32]					=>		0
		//[var_2_32] + 0							=>		[var_2_32]
		//[var_2_32] * 1							=>		[var_2_32]
		//[var_2_32] & 0xffffffff00000000{0}		=>		0x0
		//[var_2_32] & 0xffffffff{-1}				=>		[var_2_32]	
		bool processConstRightOperand(OperationalNode* opNode);

		//[sym1] & [sym1] => [sym1]
		//[sym1] | [sym1] => [sym1]
		//[sym1] ^ [sym1] => 0x0
		bool processEqualOperands(OperationalNode* opNode);

		//a << 0x2{2} => a * 4
		bool processShl(OperationalNode* opNode);
	};
};