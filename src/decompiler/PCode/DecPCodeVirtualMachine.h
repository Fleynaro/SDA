#pragma once
#include "DecPCode.h"
#include <functional>

namespace CE::Decompiler::PCode
{
	class VmException : public std::exception {
	public: VmException(const std::string& message) : std::exception(message.c_str()) {}
	};
	
	class VmExecutionContext
	{
		std::map<int, DataValue> m_registers;
		std::map<SymbolVarnode*, DataValue> m_symbolVarnodes;
	public:
		DataValue m_nextInstrAddr = 0;
		
		VmExecutionContext()
		{}

		const std::map<int, DataValue>& getRegisters() const {
			return m_registers;
		}

		const std::map<SymbolVarnode*, DataValue>& getSymbolVarnodes() const {
			return m_symbolVarnodes;
		}

		void setRegisterValue(const Register& reg, DataValue value);

		bool getRegisterValue(const Register& reg, DataValue& value) const;

		void setValue(Varnode* varnode, DataValue value);

		bool getValue(Varnode* varnode, DataValue& value) const;

		void syncWith(const std::map<int, DataValue>& registers) {
			m_registers = registers;
			m_symbolVarnodes.clear();
		}
	};

	class VmMemoryContext
	{
		std::map<std::uintptr_t, DataValue> m_values;
		std::function<bool(std::uintptr_t, DataValue&)> m_addressSpaceCallaback;
		bool m_hasCallback = false;
	public:
		VmMemoryContext()
		{}

		void setValue(std::uintptr_t address, DataValue value);

		bool getValue(std::uintptr_t address, DataValue& value);

		// after sync
		void clear() {
			m_values.clear();
		}

		void setAddressSpaceCallaback(const std::function<bool(std::uintptr_t, DataValue&)>& addressSpaceCallaback) {
			m_addressSpaceCallaback = addressSpaceCallaback;
			m_hasCallback = true;
		}
	};

	class VirtualMachine
	{
		VmExecutionContext* m_execCtx;
		VmMemoryContext* m_memCtx;
		DataValue m_op1 = 0;
		DataValue m_op2 = 0;
		DataValue m_result = 0;
		Instruction* m_instr = nullptr;
		bool m_throwException;
	public:
		VirtualMachine(VmExecutionContext* execCtx, VmMemoryContext* memCtx, bool throwException = true)
			: m_execCtx(execCtx), m_memCtx(memCtx), m_throwException(throwException)
		{}

		void execute(Instruction* instr);

	private:
		void dispatch();

		void executeDataMoving();

		void executeArithmetic();

		void executeLogical();

		void executeIntegerComp();

		void executeBoolean();

		template<typename T>
		void executeFloatingPoint() {
			T fresult = 0;
			const auto fop1 = (T&)m_op1;
			const auto fop2 = (T&)m_op2;

			switch (m_instr->m_id)
			{
			case InstructionId::FLOAT_NAN: {
				fresult = std::numeric_limits<T>::quiet_NaN();
				break;
			}
				
			case InstructionId::FLOAT_NEG: {
				fresult = -fop1;
				break;
			}
			case InstructionId::FLOAT_ABS: {
				fresult = abs(fop1);
				break;
			}
			case InstructionId::FLOAT_SQRT: {
				fresult = sqrt(fop1);
				break;
			}
				
			case InstructionId::FLOAT_ADD: {
				fresult = fop1 + fop2;
				break;
			}
			case InstructionId::FLOAT_SUB: {
				fresult = fop1 - fop2;
				break;
			}
			case InstructionId::FLOAT_MULT: {
				fresult = fop1 * fop2;
				break;
			}
			case InstructionId::FLOAT_DIV: {
				if(fop2 != 0)
					fresult = fop1 / fop2;
				// todo: throw exception
				break;
			}
			}

			(T&)m_result = fresult;
		}

		template<typename T>
		void executeFloatingPointComp() {
			const auto fop1 = (T&)m_op1;
			const auto fop2 = (T&)m_op2;
			switch (m_instr->m_id)
			{
			case InstructionId::FLOAT_EQUAL: {
				m_result = fop1 == fop2;
				break;
			}
			case InstructionId::FLOAT_NOTEQUAL: {
				m_result = fop1 != fop2;
				break;
			}
			case InstructionId::FLOAT_LESS: {
				m_result = fop1 < fop2;
				break;
			}
			case InstructionId::FLOAT_LESSEQUAL: {
				m_result = fop1 <= fop2;
				break;
			}
			}
		}

		template<typename T>
		void executeFloatingPointConversion() {
			T fresult = 0;
			const auto fop1 = (T&)m_op1;
			const auto fop2 = (T&)m_op2;
			switch (m_instr->m_id)
			{
			case InstructionId::INT2FLOAT: {
				fresult = (T)m_op1;
				break;
			}
			case InstructionId::FLOAT_CEIL: {
				fresult = ceil(fop1);
				break;
			}
			case InstructionId::FLOAT_FLOOR: {
				fresult = floor(fop1);
				break;
			}
			case InstructionId::FLOAT_ROUND: {
				fresult = round(fop1);
				break;
			}
			}

			(T&)m_result = fresult;
		}

		void executeFloatToFloat();

		void executeFloatToInt();

		void executeFloatTrunc();

		void executeBranching();

		void executeExtension();

		template<typename T_in = int, typename T_out = int64_t>
		void executeSignExtension() {
			(T_out&)m_result = (T_out)(T_in&)m_op1;
		}

		void executeTruncation();

		DataValue getValue(Varnode* varnode) const;
	};
};