#pragma once
#include <memory>
#include "PcodeVarnodes.h"
#include "Core/Offset.h"

namespace sda::pcode
{
    // Pcode instruction id
	enum InstructionId {
		NONE,
		UNKNOWN,

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

        InstructionOffset(Offset offset);

        InstructionOffset(Offset byteOffset, size_t index);

        operator size_t() const;
    };
    

    class Instruction
    {
        InstructionId m_id;
        std::shared_ptr<Varnode> m_input0;
        std::shared_ptr<Varnode> m_input1;
        std::shared_ptr<Varnode> m_output;
    public:
        Instruction(
            InstructionId id,
            std::shared_ptr<Varnode> input0,
            std::shared_ptr<Varnode> input1,
            std::shared_ptr<Varnode> output);

        InstructionId getId() const;

        std::shared_ptr<Varnode> getInput0() const;

        std::shared_ptr<Varnode> getInput1() const;

        std::shared_ptr<Varnode> getOutput() const;
    };
};