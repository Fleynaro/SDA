#include "ExprCompositeCondOptimization.h"

using namespace CE::Decompiler;

//(x < 2 || x == 2)		->		(x <= 2)

CE::Decompiler::Optimization::ExprCompositeConditionOptimization::ExprCompositeConditionOptimization(CompositeCondition* compCond)
	: ExprModification(compCond)
{}

void CE::Decompiler::Optimization::ExprCompositeConditionOptimization::start() {
	inverseConditions(getCompCondition());
	if (getCompCondition()) { // check if this node continue being comp. condition during replacing
		makeOrderInCompositeCondition(getCompCondition());
	}
	if (getCompCondition()) {
		optimizeCompositeCondition(getCompCondition());
	}
}

CompositeCondition* CE::Decompiler::Optimization::ExprCompositeConditionOptimization::getCompCondition() {
	return dynamic_cast<CompositeCondition*>(getNode());
}

//!(x == 2)	-> (x != 2)

bool CE::Decompiler::Optimization::ExprCompositeConditionOptimization::inverseConditions(CompositeCondition* compCond) {
	if (compCond->m_cond == CompositeCondition::Not) {
		compCond->m_leftCond->inverse();
		replace(compCond->m_leftCond);
		return true;
	}
	else if (compCond->m_cond == CompositeCondition::None) {
		replace(compCond->m_leftCond);
		return true;
	}
	return false;
}

//x > 2 && x == 3 -> x == 3 && x > 2

void CE::Decompiler::Optimization::ExprCompositeConditionOptimization::makeOrderInCompositeCondition(CompositeCondition* compCond) const
{
	if (!compCond->m_rightCond)
		return;

	bool isSwap = false;
	if (auto cond1 = dynamic_cast<Condition*>(compCond->m_leftCond)) {
		if (auto cond2 = dynamic_cast<Condition*>(compCond->m_rightCond)) {
			if (cond1->m_cond > cond2->m_cond) { //sorting condition
				isSwap = true;
			}
		}
		else {
			isSwap = true;
		}
	}

	if (isSwap) {
		std::swap(compCond->m_leftCond, compCond->m_rightCond);
	}

	for (auto it : { compCond->m_leftCond, compCond->m_rightCond }) {
		if (auto compCond = dynamic_cast<CompositeCondition*>(it)) {
			makeOrderInCompositeCondition(compCond);
		}
	}
}

void CE::Decompiler::Optimization::ExprCompositeConditionOptimization::optimizeCompositeCondition(CompositeCondition* compCond) {
	if (auto leftSimpleCond = dynamic_cast<Condition*>(compCond->m_leftCond)) {
		if (auto rightSimpleCond = dynamic_cast<Condition*>(compCond->m_rightCond)) {
			if (leftSimpleCond->m_leftNode->getHash().getHashValue() == rightSimpleCond->m_leftNode->getHash().getHashValue()
				&& leftSimpleCond->m_rightNode->getHash().getHashValue() == rightSimpleCond->m_rightNode->getHash().getHashValue()) {
				auto newCondType = Condition::None;
				if (compCond->m_cond == CompositeCondition::Or) {
					if (leftSimpleCond->m_cond == Condition::Eq) {
						if (rightSimpleCond->m_cond == Condition::Gt) {
							newCondType = Condition::Ge;
						}
						else if (rightSimpleCond->m_cond == Condition::Lt) {
							newCondType = Condition::Le;
						}
					}
				}
				else if (compCond->m_cond == CompositeCondition::And) {
					if (leftSimpleCond->m_cond == Condition::Ne) {
						if (rightSimpleCond->m_cond == Condition::Ge) {
							newCondType = Condition::Gt;
						}
						else if (rightSimpleCond->m_cond == Condition::Le) {
							newCondType = Condition::Lt;
						}
					}
				}

				if (newCondType != Condition::None) {
					auto newSimpleCond = new Condition(leftSimpleCond->m_leftNode, leftSimpleCond->m_rightNode, newCondType);
					replace(newSimpleCond);
				}
			}
		}
	}
}
