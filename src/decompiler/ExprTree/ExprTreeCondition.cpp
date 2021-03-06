#include "ExprTreeCondition.h"

using namespace CE::Decompiler;
using namespace ExprTree;

std::string CompositeCondition::ShowConditionType(CompositeConditionType condType) {
	switch (condType)
	{
	case And: return "&&";
	case Or: return "||";
	}
	return "_";
}

CompositeCondition::CompositeCondition(AbstractCondition* leftCond, AbstractCondition* rightCond, CompositeConditionType cond, PCode::Instruction* instr)
	: m_leftCond(leftCond), m_rightCond(rightCond), m_cond(cond), AbstractCondition(instr)
{
	leftCond->addParentNode(this);
	if (rightCond != nullptr) {
		rightCond->addParentNode(this);
	}
}

CompositeCondition::~CompositeCondition() {
	if (m_leftCond != nullptr)
		m_leftCond->removeBy(this);
	if (m_rightCond != nullptr)
		m_rightCond->removeBy(this);
}

void CompositeCondition::replaceNode(INode* node, INode* newNode) {
	if (const auto cond = dynamic_cast<AbstractCondition*>(node)) {
		if (const auto newCond = dynamic_cast<AbstractCondition*>(newNode)) {
			if (m_leftCond == cond)
				m_leftCond = newCond;
			else if (m_rightCond == cond)
				m_rightCond = newCond;
		}
	}
}

std::list<INode*> CompositeCondition::getNodesList() {
	return { m_leftCond, m_rightCond };
}

INode* CompositeCondition::clone(NodeCloneContext* ctx) {
	return new CompositeCondition(dynamic_cast<AbstractCondition*>(m_leftCond->clone(ctx)), m_rightCond ? dynamic_cast<AbstractCondition*>(m_rightCond->clone(ctx)) : nullptr, m_cond);
}

HS CompositeCondition::getHash() {
	return HS()
		<< (m_leftCond->getHash() + (m_rightCond ? m_rightCond->getHash() : 0x0))
		<< static_cast<int>(m_cond);
}

void CompositeCondition::inverse() {
	if (m_cond == Not) {
		m_cond = None;
		return;
	}
	if (m_cond == None) {
		m_cond = Not;
		return;
	}

	switch (m_cond)
	{
	case And:
		m_cond = Or;
		break;
	case Or:
		m_cond = And;
		break;
	}

	if (m_leftCond)
		m_leftCond->inverse();
	if (m_rightCond)
		m_rightCond->inverse();
}

std::string CompositeCondition::printDebug() {
	if (!m_leftCond)
		return "";
	if (m_cond == None) {
		return m_updateDebugInfo = m_leftCond->printDebug();
	}
	if (m_cond == Not) {
		return m_updateDebugInfo = ("!(" + m_leftCond->printDebug() + ")");
	}
	return m_updateDebugInfo = ("(" + m_leftCond->printDebug() + " " + ShowConditionType(m_cond) + " " + m_rightCond->printDebug() + ")");
}

Condition::Condition(INode* leftNode, INode* rightNode, ConditionType cond, PCode::Instruction* instr)
	: m_leftNode(leftNode), m_rightNode(rightNode), m_cond(cond), AbstractCondition(instr)
{
	leftNode->addParentNode(this);
	rightNode->addParentNode(this);
}

Condition::~Condition() {
	if (m_leftNode != nullptr)
		m_leftNode->removeBy(this);
	if (m_rightNode != nullptr)
		m_rightNode->removeBy(this);
}

void Condition::replaceNode(INode* node, INode* newNode) {
	if (m_leftNode == node) {
		m_leftNode = newNode;
	}
	else if (m_rightNode == node) {
		m_rightNode = newNode;
	}
}

std::list<INode*> Condition::getNodesList() {
	return { m_leftNode, m_rightNode };
}

INode* Condition::clone(NodeCloneContext* ctx) {
	return new Condition(m_leftNode->clone(ctx), m_rightNode->clone(ctx), m_cond);
}

HS Condition::getHash() {
	return HS()
		<< (m_leftNode->getHash() + m_rightNode->getHash())
		<< static_cast<int>(m_cond);
}

void Condition::inverse() {
	switch (m_cond)
	{
	case Eq:
		m_cond = Ne;
		break;
	case Ne:
		m_cond = Eq;
		break;
	case Lt:
		m_cond = Ge;
		break;
	case Le:
		m_cond = Gt;
		break;
	case Gt:
		m_cond = Le;
		break;
	case Ge:
		m_cond = Lt;
		break;
	}
}

std::string Condition::printDebug() {
	if (!m_leftNode || !m_rightNode)
		return "";
	return m_updateDebugInfo = ("(" + m_leftNode->printDebug() + " " + ShowConditionType(m_cond) + " " + m_rightNode->printDebug() + ")");
}

BooleanValue::BooleanValue(bool value, PCode::Instruction* instr)
	: m_value(value), AbstractCondition(instr)
{}

void BooleanValue::inverse() {
	m_value ^= true;
}

INode* BooleanValue::clone(NodeCloneContext* ctx) {
	return new BooleanValue(m_value);
}

HS BooleanValue::getHash() {
	return HS() << m_value;
}

std::string BooleanValue::printDebug() {
	return m_updateDebugInfo = (m_value ? "true" : "false");
}

AbstractCondition::AbstractCondition(PCode::Instruction* instr)
	: m_instr(instr)
{}

int AbstractCondition::getSize() {
	return 1;
}

std::list<PCode::Instruction*> AbstractCondition::getInstructionsRelatedTo() {
	if (m_instr)
		return { m_instr };
	return {};
}
