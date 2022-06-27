#pragma once
#include <memory>
#include <vector>
#include <set>
#include "IRcodeValue.h"
#include "Core/Pcode/PcodeInstruction.h"

namespace sda::ircode
{
    enum class OperationId {
		NONE,
		UNKNOWN,

		// Data Moving
		LOAD,

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

		// Other
        EXTRACT
	};

    class Operation
    {
        OperationId m_id = OperationId::NONE;
        std::vector<std::shared_ptr<Value>> m_inputs;
        std::shared_ptr<Variable> m_output;
        std::set<pcode::Instruction> m_pcodeInstructions;
    public:
        Operation() = default;

        OperationId getId() const;

        const std::vector<std::shared_ptr<Value>>& getInputs() const;

        std::shared_ptr<Variable> getOutput() const;

        const std::set<pcode::Instruction>& getPcodeInstructions() const;
    };
};