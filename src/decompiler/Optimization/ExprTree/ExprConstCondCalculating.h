#pragma once
#include "ExprModification.h"
#include "ExprConstCalculating.h"

namespace CE::Decompiler::Optimization
{
	// x == NaN -> false, (x >= 0) == 0 -> x < 0, x	|| true	-> true
	class ExprConstConditionCalculating : public ExprModification
	{
	public:
		ExprConstConditionCalculating(AbstractCondition* cond);

		void start() override;

		AbstractCondition* getCondition();
	private:
		bool processSimpleCondition(Condition* cond);

		bool processCompositeCondition(CompositeCondition* compCond);
	};
};