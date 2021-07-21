#include "ExprTreeOperationalNode.h"

using namespace CE::Decompiler;
using namespace ExprTree;

// groups: Arithmetic, Logic, Memory
OperationGroup ExprTree::GetOperationGroup(OperationType opType) {
	if (opType >= Add && opType <= Mod)
		return OperationGroup::Arithmetic;
	if (opType >= And && opType <= Shl)
		return OperationGroup::Logic;
	if (opType == ReadValue)
		return OperationGroup::Memory;
	return OperationGroup::None;
}

// unsupported to calculate: ReadValue, Cast, ...
bool ExprTree::IsOperationUnsupportedToCalculate(OperationType operation) {
	return operation == ReadValue || operation == Cast || operation == Functional || operation == fFunctional;
}

bool ExprTree::IsOperationFloatingPoint(OperationType operation) {
	return operation >= fAdd && operation <= fFunctional;
}

// e.g. ReadValue, Cast, ...
bool ExprTree::IsOperationWithSingleOperand(OperationType operation) {
	return operation == ReadValue || operation == Cast || operation == fFunctional;
}

// Add, Mul, Shl
bool ExprTree::IsOperationOverflow(OperationType opType) {
	return (opType == Add || opType == Mul || opType == Shl) && !IsOperationUnsupportedToCalculate(opType);
}

// in these operations their operands can be swaped (Add, Mul, ...)
bool ExprTree::IsOperationMoving(OperationType opType) {
	return !(opType == fDiv || opType == Div || opType == Shr || opType == Shl || opType == Concat || opType == Subpiece) && !IsOperationUnsupportedToCalculate(opType);
}

// Add, Mul, ...
bool ExprTree::IsOperationSigned(OperationType opType) {
	return (GetOperationGroup(opType) == OperationGroup::Arithmetic && opType != Mod) && !IsOperationUnsupportedToCalculate(opType);
}

// in these operations bits of operands can be viewed separately: And, Or, Xor
bool ExprTree::IsOperationManipulatedWithBitVector(OperationType opType) {
	return (opType == And || opType == Or || opType == Xor) && !IsOperationUnsupportedToCalculate(opType);
}

OperationalNode::OperationalNode(INode* leftNode, INode* rightNode, OperationType operation,
                                 PCode::Instruction* instr): m_leftNode(leftNode), m_rightNode(rightNode),
                                                             m_operation(operation), m_instr(instr) {
	leftNode->addParentNode(this);
	if (rightNode != nullptr) {
		rightNode->addParentNode(this);
	}
	else {
		if (!IsOperationWithSingleOperand(operation))
			throw std::logic_error("The second operand is empty in the binary operation.");
	}
}

OperationalNode::~OperationalNode() {
	auto leftNode = m_leftNode;
	if (leftNode != nullptr)
		leftNode->removeBy(this);
	if (m_rightNode != nullptr && m_rightNode != leftNode)
		m_rightNode->removeBy(this);
}

void OperationalNode::replaceNode(INode* node, INode* newNode) {
	if (m_leftNode == node) {
		m_leftNode = newNode;
	}
	else if (m_rightNode == node) {
		m_rightNode = newNode;
	}
}

OperationType OperationalNode::getOperation() {
	return m_operation;
}

std::list<INode*> OperationalNode::getNodesList() {
	return { m_leftNode, m_rightNode };
}

std::list<PCode::Instruction*> OperationalNode::getInstructionsRelatedTo() {
	if (m_instr)
		return { m_instr };
	return {};
}

HS OperationalNode::getHash() {
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

int OperationalNode::getSize() {
	if (m_operation == Concat) {
		return std::min(8, m_leftNode->getSize() + m_rightNode->getSize());
	}
	return m_leftNode->getSize();
}

bool OperationalNode::isFloatingPoint() {
	return IsOperationFloatingPoint(m_operation);
}

INode* OperationalNode::clone(NodeCloneContext* ctx) {
	return new OperationalNode(m_leftNode->clone(ctx), m_rightNode ? m_rightNode->clone(ctx) : nullptr, m_operation, m_instr);
}

std::string OperationalNode::getOpSize(int size, bool isFloat) {
	std::string opSize = "";
	if (true) {
		opSize = "." + std::to_string(size);
		if (isFloat) {
			opSize += "f";
		}
	}
	return opSize;
}

INode* ReadValueNode::getAddress() const
{
	return m_leftNode;
}

int ReadValueNode::getSize() {
	return m_size;
}

INode* ReadValueNode::clone(NodeCloneContext* ctx) {
	const auto memVar = m_memVar ? dynamic_cast<Symbol::MemoryVariable*>(m_memVar->clone(ctx)) : nullptr;
	const auto readValueNode = new ReadValueNode(m_leftNode->clone(ctx), m_size, m_instr);
	readValueNode->m_memVar = memVar;
	return readValueNode;
}

HS CastNode::getHash() {
	return OperationalNode::getHash() << m_size << m_isSigned;
}

INode* CastNode::getNode() const
{
	return m_leftNode;
}

int CastNode::getSize() {
	return m_size;
}

bool CastNode::isSigned() const
{
	return m_isSigned;
}

INode* CastNode::clone(NodeCloneContext* ctx) {
	return new CastNode(m_leftNode->clone(ctx), m_size, m_isSigned);
}

int FunctionalNode::getSize() {
	return 1;
}

HS FunctionalNode::getHash() {
	return OperationalNode::getHash() << static_cast<int>(m_funcId);
}

INode* FunctionalNode::clone(NodeCloneContext* ctx) {
	return new FunctionalNode(m_leftNode->clone(ctx), m_rightNode->clone(ctx), m_funcId, m_instr);
}

int FloatFunctionalNode::getSize() {
	return m_size;
}

HS FloatFunctionalNode::getHash() {
	return OperationalNode::getHash() << static_cast<int>(m_funcId);
}

INode* FloatFunctionalNode::getNode() const {
	return m_leftNode;
}

bool FloatFunctionalNode::isFloatingPoint() {
	return m_funcId != Id::TOINT;
}

INode* FloatFunctionalNode::clone(NodeCloneContext* ctx) {
	return new FloatFunctionalNode(m_leftNode->clone(ctx), m_funcId, m_size, m_instr);
}