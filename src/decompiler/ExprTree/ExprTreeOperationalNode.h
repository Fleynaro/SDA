#pragma once
#include "ExprTreeLeaf.h"
#include <magic_enum.hpp>

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
	extern OperationGroup GetOperationGroup(OperationType opType);

	// unsupported to calculate: ReadValue, Cast, ...
	extern bool IsOperationUnsupportedToCalculate(OperationType operation);

	extern bool IsOperationFloatingPoint(OperationType operation);

	// e.g. ReadValue, Cast, ...
	extern bool IsOperationWithSingleOperand(OperationType operation);

	// Add, Mul, Shl
	extern bool IsOperationOverflow(OperationType opType);

	// in these operations their operands can be swaped (Add, Mul, ...)
	extern bool IsOperationMoving(OperationType opType);

	// Add, Mul, ...
	extern bool IsOperationSigned(OperationType opType);

	// in these operations bits of operands can be viewed separately: And, Or, Xor
	extern bool IsOperationManipulatedWithBitVector(OperationType opType);

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

		~OperationalNode();

		void replaceNode(INode* node, INode* newNode) override;

		std::list<INode*> getNodesList() override;

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override;

		HS getHash() override;

		int getSize() override;

		bool isFloatingPoint() override;

		INode* clone(NodeCloneContext* ctx) override;

		static std::string getOpSize(int size, bool isFloat);
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

		INode* getAddress() const;

		int getSize() override;

		INode* clone(NodeCloneContext* ctx) override;
	};

	// Casting between signed and unsgined value of different size (for movsx, imul, idiv, ...)
	class CastNode : public OperationalNode
	{
		friend class ExprTreeViewGenerator;
		int m_size;
		bool m_isSigned;
	public:
		CastNode(INode* node, int size, bool isSigned)
			: OperationalNode(node, nullptr, Cast), m_size(size), m_isSigned(isSigned)
		{}

		HS getHash() override;

		INode* getNode() const;

		int getSize() override;

		bool isSigned() const;

		INode* clone(NodeCloneContext* ctx) override;
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

		int getSize() override;

		HS getHash() override;

		INode* clone(NodeCloneContext* ctx) override;
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

		int getSize() override;

		HS getHash() override;

		bool isFloatingPoint() override;

		INode* clone(NodeCloneContext* ctx) override;

	};
};