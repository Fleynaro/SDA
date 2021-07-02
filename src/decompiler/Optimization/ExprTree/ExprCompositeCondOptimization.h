#pragma once
#include "ExprModification.h"

namespace CE::Decompiler::Optimization
{
	// Simplify composite condition: !(x == 2) -> (x != 2),	x > 2 && x == 3 -> x == 3 && x > 2,		(x < 2 || x == 2) -> (x <= 2)
	class ExprCompositeConditionOptimization : public ExprModification
	{
	public:
		ExprCompositeConditionOptimization(CompositeCondition* compCond);

		void start() override;

		CompositeCondition* getCompCondition();
	private:
		//!(x == 2)	-> (x != 2)
		bool inverseConditions(CompositeCondition* compCond);

		//x > 2 && x == 3 -> x == 3 && x > 2
		void makeOrderInCompositeCondition(CompositeCondition* compCond);

		//(x < 2 || x == 2)		->		(x <= 2)
		void optimizeCompositeCondition(CompositeCondition* compCond);
	};
};