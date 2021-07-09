#include "ExprTreeOperationalNode.h"

using namespace CE::Decompiler;
using namespace CE::Decompiler::ExprTree;

// groups: Arithmetic, Logic, Memory
OperationGroup CE::Decompiler::ExprTree::GetOperationGroup(OperationType opType) {
	if (opType >= Add && opType <= Mod)
		return OperationGroup::Arithmetic;
	if (opType >= And && opType <= Shl)
		return OperationGroup::Logic;
	if (opType == ReadValue)
		return OperationGroup::Memory;
	return OperationGroup::None;
}

// unsupported to calculate: ReadValue, Cast, ...
bool CE::Decompiler::ExprTree::IsOperationUnsupportedToCalculate(OperationType operation) {
	return operation == ReadValue || operation == Cast || operation == Functional || operation == fFunctional;
}

bool CE::Decompiler::ExprTree::IsOperationFloatingPoint(OperationType operation) {
	return operation >= fAdd && operation <= fFunctional;
}

// e.g. ReadValue, Cast, ...
bool CE::Decompiler::ExprTree::IsOperationWithSingleOperand(OperationType operation) {
	return operation == ReadValue || operation == Cast || operation == fFunctional;
}

// Add, Mul, Shl
bool CE::Decompiler::ExprTree::IsOperationOverflow(OperationType opType) {
	return (opType == Add || opType == Mul || opType == Shl) && !IsOperationUnsupportedToCalculate(opType);
}

// in these operations their operands can be swaped (Add, Mul, ...)
bool CE::Decompiler::ExprTree::IsOperationMoving(OperationType opType) {
	return !(opType == fDiv || opType == Div || opType == Shr || opType == Shl || opType == Concat || opType == Subpiece) && !IsOperationUnsupportedToCalculate(opType);
}

// Add, Mul, ...
bool CE::Decompiler::ExprTree::IsOperationSigned(OperationType opType) {
	return (GetOperationGroup(opType) == OperationGroup::Arithmetic && opType != Mod) && !IsOperationUnsupportedToCalculate(opType);
}

// in these operations bits of operands can be viewed separately: And, Or, Xor
bool CE::Decompiler::ExprTree::IsOperationManipulatedWithBitVector(OperationType opType) {
	return (opType == And || opType == Or || opType == Xor) && !IsOperationUnsupportedToCalculate(opType);
}

// print operation sign
std::string CE::Decompiler::ExprTree::ShowOperation(OperationType opType) {
	switch (opType)
	{
	case Add: return "+";
	case Mul: return "*";
	case Div: return "/";
	case fAdd: return "+";
	case fMul: return "*";
	case fDiv: return "/";
	case Mod: return "%";
	case And: return "&";
	case Or: return "|";
	case Xor: return "^";
	case Shr: return ">>";
	case Shl: return "<<";
	case ReadValue: return "&";
	}
	return "_";
}

CE::Decompiler::ExprTree::OperationalNode::~OperationalNode() {
	auto leftNode = m_leftNode;
	if (leftNode != nullptr)
		leftNode->removeBy(this);
	if (m_rightNode != nullptr && m_rightNode != leftNode)
		m_rightNode->removeBy(this);
}

void CE::Decompiler::ExprTree::OperationalNode::replaceNode(INode* node, INode* newNode) {
	if (m_leftNode == node) {
		m_leftNode = newNode;
	}
	else if (m_rightNode == node) {
		m_rightNode = newNode;
	}
}

std::list<ExprTree::INode*> CE::Decompiler::ExprTree::OperationalNode::getNodesList() {
	return { m_leftNode, m_rightNode };
}

std::list<PCode::Instruction*> CE::Decompiler::ExprTree::OperationalNode::getInstructionsRelatedTo() {
	if (m_instr)
		return { m_instr };
	return {};
}

HS CE::Decompiler::ExprTree::OperationalNode::getHash() {
	auto hs = HS()
		<< static_cast<int>(m_operation)
		<< getSize()
		<< isFloatingPoint();

	const auto leftNodeHash = m_leftNode->getHash();
	const auto rightNodeHash = m_rightNode ? m_rightNode->getHash() : 0x0;
	if (IsOperationMoving(m_operation)) {
		hs = hs << (leftNodeHash + rightNodeHash);
	}
	else {
		hs = hs << (leftNodeHash << rightNodeHash);
	}
	return hs;
}

int CE::Decompiler::ExprTree::OperationalNode::getSize() {
	if (m_operation == Concat) {
		return std::min(8, m_leftNode->getSize() + m_rightNode->getSize());
	}
	return m_leftNode->getSize();
}

bool CE::Decompiler::ExprTree::OperationalNode::isFloatingPoint() {
	return IsOperationFloatingPoint(m_operation);
}

INode* CE::Decompiler::ExprTree::OperationalNode::clone(NodeCloneContext* ctx) {
	return new OperationalNode(m_leftNode->clone(ctx), m_rightNode ? m_rightNode->clone(ctx) : nullptr, m_operation, m_instr);
}

std::string CE::Decompiler::ExprTree::OperationalNode::printDebug() {
	if (!m_leftNode || !m_rightNode)
		return "";
	std::string result = "";
	const auto opSizeStr = getOpSize(getSize(), isFloatingPoint());
	if (m_operation == Xor) {
		auto numLeaf = dynamic_cast<INumberLeaf*>(m_rightNode);
		if (numLeaf && numLeaf->getValue() == -1) {
			result = "~" + m_leftNode->printDebug();
		}
	}
	if (m_operation == Concat) {
		result = "CONCAT<" + opSizeStr + ">(" + m_leftNode->printDebug() + ", " + m_rightNode->printDebug() + "; " + std::to_string(m_rightNode->getSize() * 0x8) + ")";
	}

	if (result.empty())
		result = "(" + m_leftNode->printDebug() + " " + ShowOperation(m_operation) + "" + opSizeStr + " " + m_rightNode->printDebug() + ")";
	return (m_updateDebugInfo = result);
}

std::string CE::Decompiler::ExprTree::OperationalNode::getOpSize(int size, bool isFloat) {
	std::string opSize = "";
	if (true) {
		opSize = "." + std::to_string(size);
		if (isFloat) {
			opSize += "f";
		}
	}
	return opSize;
}

INode* CE::Decompiler::ExprTree::ReadValueNode::getAddress() const
{
	return m_leftNode;
}

int CE::Decompiler::ExprTree::ReadValueNode::getSize() {
	return m_size;
}

INode* CE::Decompiler::ExprTree::ReadValueNode::clone(NodeCloneContext* ctx) {
	const auto memVar = m_memVar ? dynamic_cast<Symbol::MemoryVariable*>(m_memVar->clone(ctx)) : nullptr;
	const auto readValueNode = new ReadValueNode(m_leftNode->clone(ctx), m_size, m_instr);
	readValueNode->m_memVar = memVar;
	return readValueNode;
}

std::string CE::Decompiler::ExprTree::ReadValueNode::printDebug() {
	if (!m_leftNode)
		return "";
	return m_updateDebugInfo = ("*{uint_" + std::to_string(8 * getSize()) + "t*}" + m_leftNode->printDebug());
}

HS CE::Decompiler::ExprTree::CastNode::getHash() {
	return OperationalNode::getHash() << m_size << m_isSigned;
}

INode* CE::Decompiler::ExprTree::CastNode::getNode() const
{
	return m_leftNode;
}

int CE::Decompiler::ExprTree::CastNode::getSize() {
	return m_size;
}

bool CE::Decompiler::ExprTree::CastNode::isSigned() const
{
	return m_isSigned;
}

INode* CE::Decompiler::ExprTree::CastNode::clone(NodeCloneContext* ctx) {
	return new CastNode(m_leftNode->clone(ctx), m_size, m_isSigned);
}

std::string CE::Decompiler::ExprTree::CastNode::printDebug() {
	if (!m_leftNode)
		return "";
	return m_updateDebugInfo = ("{" + std::string(!m_isSigned ? "u" : "") + "int_" + std::to_string(8 * getSize()) + "t}" + m_leftNode->printDebug());
}

int CE::Decompiler::ExprTree::FunctionalNode::getSize() {
	return 1;
}

HS CE::Decompiler::ExprTree::FunctionalNode::getHash() {
	return OperationalNode::getHash() << static_cast<int>(m_funcId);
}

INode* CE::Decompiler::ExprTree::FunctionalNode::clone(NodeCloneContext* ctx) {
	return new FunctionalNode(m_leftNode->clone(ctx), m_rightNode->clone(ctx), m_funcId, m_instr);
}

std::string CE::Decompiler::ExprTree::FunctionalNode::printDebug() {
	if (!m_leftNode || !m_rightNode)
		return "";
	return m_updateDebugInfo = (std::string(magic_enum::enum_name(m_funcId)) + "(" + m_leftNode->printDebug() + ", " + m_rightNode->printDebug() + ")");
}

int CE::Decompiler::ExprTree::FloatFunctionalNode::getSize() {
	return m_size;
}

HS CE::Decompiler::ExprTree::FloatFunctionalNode::getHash() {
	return OperationalNode::getHash() << static_cast<int>(m_funcId);
}

bool CE::Decompiler::ExprTree::FloatFunctionalNode::isFloatingPoint() {
	return m_funcId != Id::TOINT;
}

INode* CE::Decompiler::ExprTree::FloatFunctionalNode::clone(NodeCloneContext* ctx) {
	return new FloatFunctionalNode(m_leftNode->clone(ctx), m_funcId, m_size, m_instr);
}

std::string CE::Decompiler::ExprTree::FloatFunctionalNode::printDebug() {
	if (!m_leftNode)
		return "";
	return m_updateDebugInfo = (std::string(magic_enum::enum_name(m_funcId)) + "(" + m_leftNode->printDebug() + ")");
}
