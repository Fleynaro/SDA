#pragma once
#include <memory>
#include <vector>
#include <set>
#include <map>
#include "IRcodeValue.h"
#include "SDA/Core/Pcode/PcodeInstruction.h"

namespace sda::ircode
{
    enum class OperationId {
		NONE,
		UNKNOWN,

		// Data Moving
		COPY,
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
		PHI,
        EXTRACT,
		CONCAT,
		CALL
	};
	
    class Operation
    {
        OperationId m_id = OperationId::NONE;
        std::shared_ptr<Variable> m_output;
		Block* m_block = nullptr;
        std::set<const pcode::Instruction*> m_pcodeInstructions;
		std::set<std::shared_ptr<Variable>> m_overwrittenVariables;
    public:
        Operation(
			OperationId id,
			std::shared_ptr<Variable> output);

		virtual ~Operation() = default;

		virtual Hash getHash() const = 0;

        OperationId getId() const;

		size_t getSize() const;

        std::shared_ptr<Variable> getOutput() const;

		Block* getBlock() const;

		void setBlock(Block* block);

        std::set<const pcode::Instruction*>& getPcodeInstructions();

		std::set<std::shared_ptr<Variable>>& getOverwrittenVariables();
    };

	class UnaryOperation : public Operation
	{
		std::shared_ptr<Value> m_input;
	public:
		UnaryOperation(
			OperationId id,
			std::shared_ptr<Value> input,
			std::shared_ptr<Variable> output);

		Hash getHash() const override;

		std::shared_ptr<Value> getInput() const;
	};

	class BinaryOperation : public Operation
	{
		std::shared_ptr<Value> m_input1;
		std::shared_ptr<Value> m_input2;
	public:
		BinaryOperation(
			OperationId id,
			std::shared_ptr<Value> input1,
			std::shared_ptr<Value> input2,
			std::shared_ptr<Variable> output);

		Hash getHash() const override;

		std::shared_ptr<Value> getInput1() const;

		std::shared_ptr<Value> getInput2() const;
	};

	class ExtractOperation : public UnaryOperation
	{
		size_t m_offset;
	public:
		ExtractOperation(
			std::shared_ptr<Value> input,
			size_t offset,
			std::shared_ptr<Variable> output);

		size_t getOffset() const;
	};

	class ConcatOperation : public BinaryOperation
	{
		size_t m_offset;
	public:
		ConcatOperation(
			std::shared_ptr<Value> input1,
			std::shared_ptr<Value> input2,
			size_t offset,
			std::shared_ptr<Variable> output);

		size_t getOffset() const;
	};

	class PhiOperation : public BinaryOperation
	{
	public:
		PhiOperation(
			std::shared_ptr<Value> input1,
			std::shared_ptr<Value> input2,
			std::shared_ptr<Variable> output);
	};
};