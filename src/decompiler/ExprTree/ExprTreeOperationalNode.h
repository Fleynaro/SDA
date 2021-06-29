#pragma once
#include "ExprTreeLeaf.h"

namespace CE::Decompiler::ExprTree
{
	enum OperationType
	{
		None,

		//Arithmetic
		Add,
		Mul,
		Div,
		Mod,

		//Logic
		And,
		Or,
		Xor,
		Shr,
		Shl,

		//Floating Point
		fAdd,
		fMul,
		fDiv,
		fFunctional,

		//Memory
		ReadValue,

		//Other
		Concat,
		Subpiece,
		Cast,
		Functional
	};

	enum class OperationGroup {
		None,
		Arithmetic,
		Logic,
		Memory
	};

	// groups: Arithmetic, Logic, Memory
	static OperationGroup GetOperationGroup(OperationType opType) {
		if (opType >= Add && opType <= Mod)
			return OperationGroup::Arithmetic;
		if (opType >= And && opType <= Shl)
			return OperationGroup::Logic;
		if(opType == ReadValue)
			return OperationGroup::Memory;
		return OperationGroup::None;
	}

	// unsupported to calculate: ReadValue, Cast, ...
	static bool IsOperationUnsupportedToCalculate(OperationType operation) {
		return operation == ReadValue || operation == Cast || operation == Functional || operation == fFunctional;
	}

	static bool IsOperationFloatingPoint(OperationType operation) {
		return operation >= fAdd && operation <= fFunctional;
	}

	// e.g. ReadValue, Cast, ...
	static bool IsOperationWithSingleOperand(OperationType operation) {
		return operation == ReadValue || operation == Cast || operation == fFunctional;
	}

	// Add, Mul, Shl
	static bool IsOperationOverflow(OperationType opType) {
		return (opType == Add || opType == Mul || opType == Shl) && !IsOperationUnsupportedToCalculate(opType);
	}

	// in these operations their operands can be swaped (Add, Mul, ...)
	static bool IsOperationMoving(OperationType opType) {
		return !(opType == fDiv || opType == Div || opType == Shr || opType == Shl || opType == Concat || opType == Subpiece) && !IsOperationUnsupportedToCalculate(opType);
	}

	// Add, Mul, ...
	static bool IsOperationSigned(OperationType opType) {
		return (GetOperationGroup(opType) == OperationGroup::Arithmetic && opType != Mod) && !IsOperationUnsupportedToCalculate(opType);
	}

	// in these operations bits of operands can be viewed separately: And, Or, Xor
	static bool IsOperationManipulatedWithBitVector(OperationType opType) {
		return (opType == And || opType == Or || opType == Xor) && !IsOperationUnsupportedToCalculate(opType);
	}

	// print operation sign
	static std::string ShowOperation(OperationType opType) {
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

	// Arithmetic, logic, floating or other operation
	class OperationalNode : public Node, public INodeAgregator, public PCode::IRelatedToInstruction
	{
	public:
		INode* m_leftNode;
		INode* m_rightNode;
		OperationType m_operation;
		bool m_notChangedMask;
		PCode::Instruction* m_instr;

		OperationalNode(INode* leftNode, INode* rightNode, OperationType operation, PCode::Instruction* instr = nullptr)
			: m_leftNode(leftNode), m_rightNode(rightNode), m_operation(operation), m_instr(instr)
		{
			leftNode->addParentNode(this);
			if (rightNode != nullptr) {
				rightNode->addParentNode(this);
			}
			else {
				if (!IsOperationWithSingleOperand(operation))
					throw std::logic_error("The second operand is empty in the binary operation.");
			}
		}

		~OperationalNode() {
			auto leftNode = m_leftNode;
			if (leftNode != nullptr)
				leftNode->removeBy(this);
			if (m_rightNode != nullptr && m_rightNode != leftNode)
				m_rightNode->removeBy(this);
		}

		void replaceNode(INode* node, INode* newNode) override {
			if (m_leftNode == node) {
				m_leftNode = newNode;
			}
			else if (m_rightNode == node) {
				m_rightNode = newNode;
			}
		}

		std::list<ExprTree::INode*> getNodesList() override {
			return { m_leftNode, m_rightNode };
		}

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override {
			if (m_instr)
				return { m_instr };
			return {};
		}

		HS getHash() override {
			auto hs = HS()
				<< (int)m_operation
				<< getSize()
				<< isFloatingPoint();

			auto leftNodeHash = m_leftNode->getHash();
			auto rightNodeHash = m_rightNode ? m_rightNode->getHash() : 0x0;
			if (IsOperationMoving(m_operation)) {
				hs = hs << (leftNodeHash + rightNodeHash);
			}
			else {
				hs = hs << (leftNodeHash << rightNodeHash);
			}
			return hs;
		}

		int getSize() override {
			if (m_operation == Concat) {
				return min(8, m_leftNode->getSize() + m_rightNode->getSize());
			}
			return m_leftNode->getSize();
		}

		bool isFloatingPoint() override {
			return IsOperationFloatingPoint(m_operation);
		}

		INode* clone(NodeCloneContext* ctx) override {
			return new OperationalNode(m_leftNode->clone(ctx), m_rightNode ? m_rightNode->clone(ctx) : nullptr, m_operation, m_instr);
		}

		std::string printDebug() override {
			if (!m_leftNode || !m_rightNode)
				return "";
			std::string result = "";
			auto opSizeStr = getOpSize(getSize(), isFloatingPoint());
			if (m_operation == Xor) {
				auto numLeaf = dynamic_cast<INumberLeaf*>(m_rightNode);
				if (numLeaf && numLeaf->getValue() == -1) {
					result = "~" + m_leftNode->printDebug();
				}
			}
			if (m_operation == Concat) {
				result = "CONCAT<"+ opSizeStr +">(" + m_leftNode->printDebug() + ", " + m_rightNode->printDebug() + "; " + std::to_string(m_rightNode->getSize() * 0x8) +")";
			}
			
			if(result.empty())
				result = "(" + m_leftNode->printDebug() + " " + ShowOperation(m_operation) + ""+ opSizeStr +" " + m_rightNode->printDebug() + ")";
			return (m_updateDebugInfo = result);
		}

		static std::string getOpSize(int size, bool isFloat) {
			std::string opSize = "";
			if (true) {
				opSize = "." + std::to_string(size);
				if (isFloat) {
					opSize += "f";
				}
			}
			return opSize;
		}
	};

	// Reads value of some size(in bytes) from the specified memory location
	class ReadValueNode : public OperationalNode
	{
		int m_size;
	public:
		Symbol::MemoryVariable* m_memVar = nullptr;

		ReadValueNode(INode* node, int size, PCode::Instruction* instr = nullptr)
			: OperationalNode(node, nullptr, ReadValue, instr), m_size(size)
		{}

		INode* getAddress() {
			return m_leftNode;
		}

		int getSize() override {
			return m_size;
		}

		INode* clone(NodeCloneContext* ctx) override {
			auto memVar = m_memVar ? dynamic_cast<Symbol::MemoryVariable*>(m_memVar->clone(ctx)) : nullptr;
			auto readValueNode = new ReadValueNode(m_leftNode->clone(ctx), m_size, m_instr);
			readValueNode->m_memVar = memVar;
			return readValueNode;
		}

		std::string printDebug() override {
			if (!m_leftNode)
				return "";
			return m_updateDebugInfo = ("*{uint_" + std::to_string(8 * getSize()) + "t*}" + m_leftNode->printDebug());
		}
	};

	// Casting between signed and unsgined value of different size (for movsx, imul, idiv, ...)
	class CastNode : public OperationalNode
	{
		int m_size;
		bool m_isSigned;
	public:
		CastNode(INode* node, int size, bool isSigned)
			: OperationalNode(node, nullptr, Cast), m_size(size), m_isSigned(isSigned)
		{}

		HS getHash() override {
			return OperationalNode::getHash() << m_size << m_isSigned;
		}

		INode* getNode() {
			return m_leftNode;
		}

		int getSize() override {
			return m_size;
		}

		bool isSigned() {
			return m_isSigned;
		}

		INode* clone(NodeCloneContext* ctx) override {
			return new CastNode(m_leftNode->clone(ctx), m_size, m_isSigned);
		}

		std::string printDebug() override {
			if (!m_leftNode)
				return "";
			return m_updateDebugInfo = ("{"+ std::string(!m_isSigned ? "u" : "") +"int_" + std::to_string(8 * getSize()) + "t}" + m_leftNode->printDebug());
		}
	};

	// Gets two arguments (float) and returns boolean value: CARRY, SCARRY, SBORROW
	class FunctionalNode : public OperationalNode
	{
	public:
		enum class Id {
			CARRY,
			SCARRY,
			SBORROW
		};
		Id m_funcId;

		FunctionalNode(INode* node1, INode* node2, Id id, PCode::Instruction* instr = nullptr)
			: OperationalNode(node1, node2, Functional, instr), m_funcId(id)
		{}

		int getSize() override {
			return 1;
		}

		HS getHash() override {
			return OperationalNode::getHash() << (int)m_funcId;
		}

		INode* clone(NodeCloneContext* ctx) override {
			return new FunctionalNode(m_leftNode->clone(ctx), m_rightNode->clone(ctx), m_funcId, m_instr);
		}

		std::string printDebug() override {
			if (!m_leftNode || !m_rightNode)
				return "";
			return m_updateDebugInfo = (std::string(magic_enum::enum_name(m_funcId)) + "(" + m_leftNode->printDebug() + ", " + m_rightNode->printDebug() + ")");
		}
	};

	// Gets one argument (float or int) and returns float value: FABS, FSQRT, FLOOR, TOFLOAT, ...
	class FloatFunctionalNode : public OperationalNode
	{
		int m_size;
	public:
		enum class Id {
			FABS,
			FSQRT,
			TRUNC,
			CEIL,
			FLOOR,
			ROUND,
			TOFLOAT,
			TOINT //todo: what is size of the result?
		};
		Id m_funcId;

		FloatFunctionalNode(INode* node1, Id id, int size, PCode::Instruction* instr = nullptr)
			: OperationalNode(node1, nullptr, fFunctional, instr), m_funcId(id), m_size(size)
		{}

		int getSize() override {
			return m_size;
		}

		HS getHash() override {
			return OperationalNode::getHash() << (int)m_funcId;
		}

		bool isFloatingPoint() override {
			return m_funcId != Id::TOINT;
		}

		INode* clone(NodeCloneContext* ctx) override {
			return new FloatFunctionalNode(m_leftNode->clone(ctx), m_funcId, m_size, m_instr);
		}

		std::string printDebug() override {
			if (!m_leftNode)
				return "";
			return m_updateDebugInfo = (std::string(magic_enum::enum_name(m_funcId)) + "(" + m_leftNode->printDebug() + ")");
		}

	};
};