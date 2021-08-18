#pragma once
#include <decompiler/DecMask.h>
#include <Zydis/Zydis.h>
#include "Offset.h"
#include "utilities/Helper.h"
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
		friend class InstructionViewGenerator;
	public:
		enum class Type {
			Generic,
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
		std::string m_debugName;
		BitMask64 m_valueRangeMask; // range of possible values

		Register(RegisterId genericId = 0, int index = 0, BitMask64 valueRangeMask = 0x0, Type type = Type::Generic);

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
	};

	// that is the feature of x86: setting value to EAX cleans fully RAX
	extern BitMask64 GetValueRangeMaskWithException(const Register& reg);

	// Register, variable(symbol) or constant (used as input or output for pCode instructions)
	class Varnode
	{
	public:
		virtual ~Varnode() {}

		virtual int getSize() = 0;

		virtual BitMask64 getMask() {
			return BitMask64(getSize());
		}
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
	};

	// e.g. 100
	class ConstantVarnode : public Varnode
	{
	public:
		uint64_t m_value;
		int m_size;
		bool m_isAddr;

		ConstantVarnode() = default;

		ConstantVarnode(uint64_t value, int size, bool isAddr)
			: m_value(value), m_size(size), m_isAddr(isAddr)
		{}

		int getSize() override;
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
		FLOAT_CEIL,
		FLOAT_FLOOR,
		FLOAT_ROUND,
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
			Offset m_offset;
			int m_length;
			std::map<int, Instruction> m_pcodeInstructions;
			std::string m_originalView;

			OriginalInstruction() = default;

			OriginalInstruction(Offset offset, int length)
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

		// get complex offset which consist of original offset and pCode instruction order number: origOffset{24} | order{8}
		ComplexOffset getOffset() const;

		// get complex offset of the next instruction following this
		ComplexOffset getFirstInstrOffsetInNextOrigInstr() const;

		// BRANCH, CBRANCH, BRANCHIND
		static bool IsBranching(InstructionId id);

		// check if the instruction is some kind of jump (BRANCH/CALL/RETURN)
		static bool IsAnyJmup(InstructionId id);
	};

	// sort instruction list
	extern void Sort(std::list<Instruction*>& instructions);

	// todo: move into operational node (assignment, condition, operations)
	class IRelatedToInstruction
	{
	public:
		virtual std::list<Instruction*> getInstructionsRelatedTo() = 0;
	};

	using DataValue = uint64_t;

	class InstructionViewGenerator
	{
	protected:
		enum TokenType
		{
			TOKEN_MNEMONIC,
			TOKEN_REGISTER,
			TOKEN_VARIABLE,
			TOKEN_NUMBER,
			TOKEN_OTHER
		};
		
	public:
		virtual void generate(const Instruction* instruction) {
			if (instruction->m_output) {
				generateVarnode(instruction->m_output);
				generateToken(" = ", TOKEN_OTHER);
			}
			
			generateToken(magic_enum::enum_name(instruction->m_id).data(), TOKEN_MNEMONIC);
			
			if (instruction->m_input0) {
				generateToken(" ", TOKEN_OTHER);
				generateVarnode(instruction->m_input0);
			}
			if (instruction->m_input1) {
				generateToken(", ", TOKEN_OTHER);
				generateVarnode(instruction->m_input1);
			}
		}

		virtual void generateVarnode(Varnode* varnode) {
			if(const auto registerVarnode = dynamic_cast<RegisterVarnode*>(varnode)) {
				const auto regName = GenerateRegisterName(registerVarnode->m_register);
				generateToken(regName, TOKEN_REGISTER);
			}
			else if (const auto symbolVarnode = dynamic_cast<SymbolVarnode*>(varnode)) {
				const auto uniqueId = (uint64_t)symbolVarnode % 10000;
				const auto symbolName = "$U" + std::to_string(uniqueId) + ":" + std::to_string(symbolVarnode->getSize());
				generateToken(symbolName, TOKEN_VARIABLE);
			}
			else if (const auto constVarnode = dynamic_cast<ConstantVarnode*>(varnode)) {
				const auto number = Helper::String::NumberToHex(constVarnode->m_value) + ":" + (constVarnode->m_isAddr ? "addr" : std::to_string(constVarnode->getSize()));
				generateToken(number, TOKEN_NUMBER);
			}
		}

		virtual void generateToken(const std::string& text, TokenType tokenType) = 0;

		static std::string GetRegisterName(int genericId) {
			return ZydisRegisterGetString(static_cast<ZydisRegister>(genericId));
		}

		static std::string GetFlagName(int flag) {
			std::string flagName = "flag";
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
			return flagName;
		}

		static std::string GenerateRegisterName(const Register& reg) {
			const auto regId = static_cast<ZydisRegister>(reg.m_genericId);

			const auto size = reg.getSize();
			std::string maskStr = std::to_string(size);
			if (reg.isVector()) {
				if (size == 4 || size == 8) {
					maskStr = std::string(size == 4 ? "D" : "Q") + static_cast<char>('a' + static_cast<char>(reg.getOffset() / (size * 8)));
				}
			}

			if (regId != ZYDIS_REGISTER_RFLAGS)
				return std::string(GetRegisterName(regId)) + ":" + maskStr;

			const auto flag = static_cast<ZydisCPUFlag>(reg.m_valueRangeMask.getOffset());
			return GetFlagName(flag) + ":1";
		}
	};

	class InstructionTextGenerator : public InstructionViewGenerator
	{
	public:
		std::string m_text;

		void generateToken(const std::string& text, TokenType tokenType) override {
			m_text += text;
		}

		std::string getText(const Instruction* instruction) {
			m_text.clear();
			generate(instruction);
			return m_text;
		}
	};
};