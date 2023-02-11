#pragma once
#include <memory>
#include <magic_enum.hpp>
#include "PcodeVarnodes.h"
#include "SDA/Core/Offset.h"

namespace sda::pcode
{
    // Pcode instruction id
	enum class InstructionId {
		NONE,
		UNKNOWN,
		NOP,

		// Data Moving
		COPY,
		LOAD,
		STORE,

		// Arithmetic
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

		// Logical
		INT_NEGATE,
		INT_XOR,
		INT_AND,
		INT_OR,
		INT_LEFT,
		INT_RIGHT,
		INT_SRIGHT,

		// Integer Comparison
		INT_EQUAL,
		INT_NOTEQUAL,
		INT_SLESS,
		INT_SLESSEQUAL,
		INT_LESS,
		INT_LESSEQUAL,

		// Boolean
		BOOL_NEGATE,
		BOOL_XOR,
		BOOL_AND,
		BOOL_OR,

		// Floating point
		FLOAT_ADD,
		FLOAT_SUB,
		FLOAT_MULT,
		FLOAT_DIV,
		FLOAT_NEG,
		FLOAT_ABS,
		FLOAT_SQRT,

		// Floating point compare
		FLOAT_NAN,
		FLOAT_EQUAL,
		FLOAT_NOTEQUAL,
		FLOAT_LESS,
		FLOAT_LESSEQUAL,

		// Floating point conversion
		INT2FLOAT,
		FLOAT2INT,
		FLOAT2FLOAT,
		TRUNC,
		FLOAT_CEIL,
		FLOAT_FLOOR,
		FLOAT_ROUND,

		// Branching
		BRANCH,
		CBRANCH,
		BRANCHIND,
		CALL,
		CALLIND,
		RETURN,
        
		// Extension / truncation
		INT_ZEXT,
		INT_SEXT,
		PIECE,
		SUBPIECE,
        
		// Managed Code
		CPOOLREF,
		NEW,

		// Interruption
		INT
	};

    struct InstructionOffset
    {
        union {
			struct {
				Offset index : 8;
				Offset byteOffset : 56;
			};
			struct {
				Offset fullOffset : 64;
			};
		};

        InstructionOffset(Offset offset = InvalidOffset);

        InstructionOffset(Offset byteOffset, size_t index);

        operator size_t() const;

		static size_t GetMaxIndex();
    };
    
    class Instruction
    {
        InstructionId m_id = InstructionId::NONE;
        std::shared_ptr<Varnode> m_input0;
        std::shared_ptr<Varnode> m_input1;
        std::shared_ptr<Varnode> m_output;
		InstructionOffset m_offset = 0;
    public:
		Instruction() = default;

        Instruction(
            InstructionId id,
            std::shared_ptr<Varnode> input0,
            std::shared_ptr<Varnode> input1,
            std::shared_ptr<Varnode> output,
			InstructionOffset offset);

        InstructionId getId() const;

        std::shared_ptr<Varnode> getInput0() const;

        std::shared_ptr<Varnode> getInput1() const;

        std::shared_ptr<Varnode> getOutput() const;

		InstructionOffset getOffset() const;

		// Check if the instruction is some kind of jump (BRANCH/CALL/RETURN)
		bool isAnyJump() const;

		// Check if the instruction is some kind of branching (BRANCH, CBRANCH, BRANCHIND)
		bool isBranching() const;

		// Check if the instruction has comutative operands (INT_ADD, INT_MULT, INT_XOR, INT_AND, INT_OR)
		bool isComutative() const;

		// Check if the instruction is not going to the next instruction (BRANCH, RETURN, INT)
		bool isNotGoingNext() const;
    };
};