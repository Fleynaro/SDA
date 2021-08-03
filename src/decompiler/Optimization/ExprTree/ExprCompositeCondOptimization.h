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

		CompositeCondition* getCompCondition() const;
	private:
		//!(x == 2)	-> (x != 2)
		bool inverseConditions();

		//x > 2 && x == 3 -> x == 3 && x > 2
		static void MakeOrderInCompositeCondition(CompositeCondition* compCond);

		//(x < 2 || x == 2)		->		(x <= 2)
		void optimizeCompositeCondition();
	};
};