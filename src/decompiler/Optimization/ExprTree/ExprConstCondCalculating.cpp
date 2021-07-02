#include "ExprConstCondCalculating.h"

using namespace CE::Decompiler;

CE::Decompiler::Optimization::ExprConstConditionCalculating::ExprConstConditionCalculating(AbstractCondition* cond)
	: ExprModification(cond)
{}

void CE::Decompiler::Optimization::ExprConstConditionCalculating::start() {
	if (auto cond = dynamic_cast<Condition*>(getCondition())) {
		processSimpleCondition(cond);
	}
	else if (auto compCond = dynamic_cast<CompositeCondition*>(getCondition())) {
		processCompositeCondition(compCond);
	}
}

AbstractCondition* CE::Decompiler::Optimization::ExprConstConditionCalculating::getCondition() {
	return dynamic_cast<AbstractCondition*>(getNode());
}

bool CE::Decompiler::Optimization::ExprConstConditionCalculating::processSimpleCondition(Condition* cond) {
	//[symbol] == NaN -> false
	if (auto floatNanLeaf = dynamic_cast<FloatNanLeaf*>(cond->m_rightNode)) {
		auto newCond = new BooleanValue(cond->m_cond == Condition::Ne);
		replace(newCond);
		return true;
	}

	//([symbol] >= 0) == 0 -> [symbol] < 0
	if (auto subCond = dynamic_cast<AbstractCondition*>(cond->m_leftNode)) {
		if (auto numberLeaf = dynamic_cast<INumberLeaf*>(cond->m_rightNode)) {
			if (numberLeaf->getValue() == 0x0 && (cond->m_cond == Condition::Eq || cond->m_cond == Condition::Ne)) {
				auto newCond = subCond;
				if (cond->m_cond == Condition::Eq)
					newCond->inverse();
				replace(newCond);
				return true;
			}
		}
	}
	return false;
}

bool CE::Decompiler::Optimization::ExprConstConditionCalculating::processCompositeCondition(CompositeCondition* compCond) {
	if (compCond->m_cond == CompositeCondition::Not || compCond->m_cond == CompositeCondition::None) {
		//!false -> true
		if (auto booleanVal = dynamic_cast<BooleanValue*>(compCond->m_leftCond)) {
			auto newCond = new BooleanValue(compCond->m_cond == CompositeCondition::None ? booleanVal->m_value : !booleanVal->m_value);
			replace(newCond);
			return true;
		}
	}

	AbstractCondition* newCond = nullptr;
	AbstractCondition* conds[2] = { compCond->m_leftCond, compCond->m_rightCond };
	bool val[2] = { false, false };
	bool val_calc[2] = { false, false };
	for (int idx = 0; idx < 2; idx++) {
		if (auto booleanVal = dynamic_cast<BooleanValue*>(conds[idx])) {
			val[idx] = booleanVal->m_value;
			val_calc[idx] = true;
		}
	}

	if (val_calc[0] || val_calc[1]) {
		//true && false		->		false
		if (val_calc[0] && val_calc[1]) {
			bool result = (compCond->m_cond == CompositeCondition::Or ? (val[0] || val[1]) : (val[0] && val[1]));
			newCond = new BooleanValue(result);
		}
		else {
			//cond1	|| true		->		true
			//cond1 || false	->		cond1
			for (int idx = 0; idx < 2; idx++) {
				if (val_calc[idx]) {
					if (compCond->m_cond == CompositeCondition::Or) {
						if (val[idx]) {
							newCond = new BooleanValue(true);
						}
						else {
							newCond = conds[1 - idx];
						}
					}
					else {
						if (!val[idx]) {
							newCond = new BooleanValue(false);
						}
						else {
							newCond = conds[1 - idx];
						}
					}
					break;
				}
			}
		}

		replace(newCond);
		return true;
	}
	return false;
}
