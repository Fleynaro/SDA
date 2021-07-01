#pragma once
#include <decompiler/DecMask.h>

//for debug x86
#include <Zycore/Format.h>
#include <Zycore/LibC.h>
#include <Zydis/Zydis.h>

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

		Type getType() const {
			return m_type;
		}

		int getId() const {
			return (m_genericId << 8) | m_index;
		}

		RegisterId getGenericId() const {
			return m_genericId;
		}

		int getIndex() const {
			return m_index;
		}

		bool isValid() const {
			return m_genericId != 0;
		}

		bool isPointer() const {
			return m_type == Type::StackPointer || m_type == Type::InstructionPointer;
		}

		bool isVector() const {
			return m_type == Type::Vector;
		}

		// get size (in bytes) of values range
		int getSize() const {
			return m_valueRangeMask.getSize();
		}

		int getOffset() const {
			return m_valueRangeMask.getOffset() + m_index * 64;
		}

		// check if memory area of two registers intersected
		bool intersect(const Register& reg) const {
			//if the masks intersected
			return getId() == reg.getId() && !(m_valueRangeMask & reg.m_valueRangeMask).isZero();
		}

		bool operator ==(const Register& reg) const {
			return getId() == reg.getId() && m_valueRangeMask == reg.m_valueRangeMask;
		}

		std::string printDebug() {
			auto regId = (ZydisRegister)m_genericId;

			auto size = getSize();
			std::string maskStr = std::to_string(size);
			if (isVector()) {
				if (size == 4 || size == 8) {
					maskStr = std::string(size == 4 ? "D" : "Q") + (char)('a' + (char)(getOffset() / (size * 8)));
				}
			}

			if (regId != ZYDIS_REGISTER_RFLAGS)
				return std::string(ZydisRegisterGetString(regId)) + ":" + maskStr;

			std::string flagName = "flag";
			auto flag = (ZydisCPUFlag)m_valueRangeMask.getOffset();
			if (flag == ZYDIS_CPUFLAG_CF)
				flagName = "CF";
			else if (flag == ZYDIS_CPUFLAG_OF)
				flagName = "OF";
			else if (flag == ZYDIS_CPUFLAG_SF)
				flagName = "SF";
			else if (flag == ZYDIS_CPUFLAG_ZF)
				flagName = "ZF";
			else if (flag == ZYDIS_CPUFLAG_AF)
				flagName = "AF";
			else if (flag == ZYDIS_CPUFLAG_PF)
				flagName = "PF";
			return flagName + ":1";
		}
	};

	// that is the feature of x86: setting value to EAX cleans fully RAX
	static BitMask64 GetValueRangeMaskWithException(const PCode::Register& reg) {
		if (reg.getType() == Register::Type::Helper && reg.m_valueRangeMask == BitMask64(4))
			return BitMask64(8);
		return reg.m_valueRangeMask;
	}

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

		int getSize() override {
			return m_register.getSize();
		}

		BitMask64 getMask() override {
			return m_register.m_valueRangeMask;
		}

		std::string printDebug() override {
			return m_register.printDebug();
		}
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

		int getSize() override {
			return m_size;
		}

		std::string printDebug() override {
			return std::to_string((int64_t&)m_value) + ":" + std::to_string(getSize());
		}
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

		int getSize() override {
			return m_size;
		}

		std::string printDebug() override {
			return "$U" + std::to_string((uint64_t)this % 10000) + ":" + std::to_string(getSize());
		}
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
		int64_t getOffset() {
			return (m_origInstruction->m_offset << 8) | m_orderId;
		}

		// get long offset of the next instruction following this
		int64_t getFirstInstrOffsetInNextOrigInstr() {
			return (m_origInstruction->m_offset + m_origInstruction->m_length) << 8;
		}

		std::string printDebug() {
			std::string result;
			if (m_output)
				result += m_output->printDebug() + " = ";
			result += magic_enum::enum_name(m_id);
			if (m_input0)
				result += " " + m_input0->printDebug();
			if (m_input1)
				result += ", " + m_input1->printDebug();
			return result;
		}

		// BRANCH, CBRANCH, BRANCHIND
		static bool IsBranching(InstructionId id) {
			return id >= InstructionId::BRANCH && id <= InstructionId::BRANCHIND;
		}

		// check if the instruction is some kind of jump (BRANCH/CALL/RETURN)
		static bool IsAnyJmup(InstructionId id) {
			return id >= InstructionId::BRANCH && id <= InstructionId::RETURN;
		}
	};

	class IRelatedToInstruction
	{
	public:
		virtual std::list<PCode::Instruction*> getInstructionsRelatedTo() = 0;
	};

	using DataValue = uint64_t;
};