#pragma once
#include <decompiler/DecMask.h>
#include <Zydis/Zydis.h>
#include <magic_enum.hpp>
#include <string>
#include <list>
#include <map>

namespace CE::Decompiler::PCode
{
	using RegisterId = int;

	// PCode register of some type (e.g. RAX:4, ZMM0:8)
	class Register
	{
	public:
		enum class Type {
			Helper,
			StackPointer,
			InstructionPointer,
			Flag,
			Vector
		};

	private:
		RegisterId m_genericId;
		int m_index; // for SIMD registers (XMM, YMM, ZMM)
		Type m_type;

	public:
		std::string m_debugInfo;
		BitMask64 m_valueRangeMask; // range of possible values

		Register(RegisterId genericId = 0, int index = 0, BitMask64 valueRangeMask = 0x0, Type type = Type::Helper)
			: m_genericId(genericId), m_index(index), m_valueRangeMask(valueRangeMask), m_type(type)
		{
			m_debugInfo = printDebug();
		}

		Type getType() const;

		int getId() const;

		RegisterId getGenericId() const;

		int getIndex() const;

		bool isValid() const;

		bool isPointer() const;

		bool isVector() const;

		// get size (in bytes) of values range
		int getSize() const;

		int getOffset() const;

		// check if memory area of two registers intersected
		bool intersect(const Register& reg) const;

		bool operator ==(const Register& reg) const {
			return getId() == reg.getId() && m_valueRangeMask == reg.m_valueRangeMask;
		}

		std::string printDebug() const;
	};

	// that is the feature of x86: setting value to EAX cleans fully RAX
	extern BitMask64 GetValueRangeMaskWithException(const PCode::Register& reg);

	// Register, variable(symbol) or constant (used as input or output for pCode instructions)
	class Varnode
	{
	public:
		virtual ~Varnode() {}

		virtual int getSize() = 0;

		virtual BitMask64 getMask() {
			return BitMask64(getSize());
		}

		virtual std::string printDebug() = 0;
	};

	// e.g. EAX, ZMM0, ...
	class RegisterVarnode : public Varnode
	{
	public:
		Register m_register;

		RegisterVarnode() = default;

		RegisterVarnode(Register reg)
			: m_register(reg)
		{}

		int getSize() override;

		BitMask64 getMask() override;

		std::string printDebug() override;
	};

	// e.g. 100
	class ConstantVarnode : public Varnode
	{
	public:
		uint64_t m_value;
		int m_size;

		ConstantVarnode() = default;

		ConstantVarnode(uint64_t value, int size)
			: m_value(value), m_size(size)
		{}

		int getSize() override;

		std::string printDebug() override;
	};

	// e.g. $U9680
	class SymbolVarnode : public Varnode
	{
	public:
		int m_size;

		SymbolVarnode() = default;

		SymbolVarnode(int size)
			: m_size(size)
		{}

		int getSize() override;

		std::string printDebug() override;
	};

	// PCode instruction id
	enum class InstructionId {
		NONE,
		UNKNOWN,
		//Data Moving
		COPY,
		LOAD,
		STORE,
		//Arithmetic
		INT_ADD,
		INT_SUB,
		INT_CARRY,
		INT_SCARRY,
		INT_SBORROW,
		INT_2COMP,
		INT_MULT,
		INT_DIV,
		INT_SDIV,
		INT_REM,
		INT_SREM,
		//Logical
		INT_NEGATE,
		INT_XOR,
		INT_AND,
		INT_OR,
		INT_LEFT,
		INT_RIGHT,
		INT_SRIGHT,
		//Integer Comparison
		INT_EQUAL,
		INT_NOTEQUAL,
		INT_SLESS,
		INT_SLESSEQUAL,
		INT_LESS,
		INT_LESSEQUAL,
		//Boolean
		BOOL_NEGATE,
		BOOL_XOR,
		BOOL_AND,
		BOOL_OR,
		//Floating Point
		FLOAT_ADD,
		FLOAT_SUB,
		FLOAT_MULT,
		FLOAT_DIV,
		FLOAT_NEG,
		FLOAT_ABS,
		FLOAT_SQRT,
		FLOAT_NAN,
		//Floating Point Compare
		FLOAT_EQUAL,
		FLOAT_NOTEQUAL,
		FLOAT_LESS,
		FLOAT_LESSEQUAL,
		//Floating Point Conversion
		INT2FLOAT,
		FLOAT2INT,
		FLOAT2FLOAT,
		TRUNC,
		CEIL,
		FLOOR,
		ROUND,
		//Branching
		BRANCH,
		CBRANCH,
		BRANCHIND,
		CALL,
		CALLIND,
		RETURN,
		//Extension / Truncation
		INT_ZEXT,
		INT_SEXT,
		PIECE,
		SUBPIECE,
		//Managed Code
		CPOOLREF,
		NEW
	};

	// PCode instruction (e.g. result = SUM op1, op2)
	class Instruction
	{
	public:
		struct OriginalInstruction {
			int64_t m_offset;
			int m_length;
			std::map<int, Instruction> m_pcodeInstructions;
			std::string m_originalView;

			OriginalInstruction() = default;

			OriginalInstruction(int64_t offset, int length)
				: m_offset(offset), m_length(length)
			{}
		};

		InstructionId m_id;
		Varnode* m_input0; // the first operand
		Varnode* m_input1; // the second operand 
		Varnode* m_output; // the result
		OriginalInstruction* m_origInstruction;
		int m_orderId;

		Instruction() = default;
		
		Instruction(InstructionId id, Varnode* input0, Varnode* input1, Varnode* output, OriginalInstruction* origInstruction, int orderId=0)
			: m_id(id), m_input0(input0), m_input1(input1), m_output(output), m_origInstruction(origInstruction), m_orderId(orderId)
		{}

		// get long offset which consist of original offset and pCode instruction order number: origOffset{24} | order{8}
		int64_t getOffset() const;

		// get long offset of the next instruction following this
		int64_t getFirstInstrOffsetInNextOrigInstr() const;

		std::string printDebug() const;

		// BRANCH, CBRANCH, BRANCHIND
		static bool IsBranching(InstructionId id);

		// check if the instruction is some kind of jump (BRANCH/CALL/RETURN)
		static bool IsAnyJmup(InstructionId id);
	};

	class IRelatedToInstruction
	{
	public:
		virtual std::list<PCode::Instruction*> getInstructionsRelatedTo() = 0;
	};

	using DataValue = uint64_t;
};