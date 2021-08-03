#include "ExprCompositeCondOptimization.h"

using namespace CE::Decompiler;

//(x < 2 || x == 2)		->		(x <= 2)

Optimization::ExprCompositeConditionOptimization::ExprCompositeConditionOptimization(CompositeCondition* compCond)
	: ExprModification(compCond)
{}

void Optimization::ExprCompositeConditionOptimization::start() {
	inverseConditions();
	if (getCompCondition()) { // check if this node continue being comp. condition during replacing
		MakeOrderInCompositeCondition(getCompCondition());
	}
	if (getCompCondition()) {
		optimizeCompositeCondition();
	}
}

CompositeCondition* Optimization::ExprCompositeConditionOptimization::getCompCondition() const {
	return dynamic_cast<CompositeCondition*>(getNode());
}

//!(x == 2)	-> (x != 2)

bool Optimization::ExprCompositeConditionOptimization::inverseConditions() {
	const auto compCond = getCompCondition();
	if (compCond->m_cond == CompositeCondition::Not) {
		compCond->m_leftCond->inverse();
		compCond->m_leftCond->addInstructions(compCond->getInstructionsRelatedTo());
		replace(compCond->m_leftCond);
		return true;
	}
	if (compCond->m_cond == CompositeCondition::None) {
		compCond->m_leftCond->addInstructions(compCond->getInstructionsRelatedTo());
		replace(compCond->m_leftCond);
		return true;
	}
	return false;
}

//x > 2 && x == 3 -> x == 3 && x > 2

void Optimization::ExprCompositeConditionOptimization::MakeOrderInCompositeCondition(CompositeCondition* compCond)
{
	if (!compCond->m_rightCond)
		return;

	bool isSwap = false;
	if (const auto cond1 = dynamic_cast<Condition*>(compCond->m_leftCond)) {
		if (const auto cond2 = dynamic_cast<Condition*>(compCond->m_rightCond)) {
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
		if (const auto compCond = dynamic_cast<CompositeCondition*>(it)) {
			MakeOrderInCompositeCondition(compCond);
		}
	}
}

void Optimization::ExprCompositeConditionOptimization::optimizeCompositeCondition() {
	const auto compCond = getCompCondition();
	
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
					const auto newSimpleCond = new Condition(leftSimpleCond->m_leftNode, leftSimpleCond->m_rightNode, newCondType, false);
					std::list<PCode::Instruction*> instructions;
					GetInstructions(rightSimpleCond, instructions);
					newSimpleCond->addInstructions(instructions);
					newSimpleCond->addInstructions(leftSimpleCond->getInstructionsRelatedTo(), true);
					newSimpleCond->addInstructions(compCond->getInstructionsRelatedTo());
					replace(newSimpleCond);
				}
			}
		}
	}
}
