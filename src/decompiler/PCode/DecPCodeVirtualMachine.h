#pragma once
#include "DecPCode.h"

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
		DataValue m_nextInstrOffset = 0;
		
		VmExecutionContext()
		{}

		void setValue(Varnode* varnode, DataValue value) {
			if (const auto varnodeRegister = dynamic_cast<RegisterVarnode*>(varnode)) {
				const auto reg = varnodeRegister->m_register;
				auto& dataCell = m_registers[reg.getId()];
				dataCell &= ~GetValueRangeMaskWithException(reg).getValue();
				dataCell |= value << reg.m_valueRangeMask.getOffset();
			}
			else if (const auto varnodeSymbol = dynamic_cast<SymbolVarnode*>(varnode)) {
				m_symbolVarnodes[varnodeSymbol] = value;
			}
		}

		DataValue getValue(Varnode* varnode) const {
			if (const auto varnodeRegister = dynamic_cast<RegisterVarnode*>(varnode)) {
				const auto& reg = varnodeRegister->m_register;
				const auto it = m_registers.find(reg.getId());
				if (it != m_registers.end()) {
					const auto value = it->second & GetValueRangeMaskWithException(reg).getValue();
					return value >> reg.m_valueRangeMask.getOffset();
				}
			}
			else if (const auto varnodeSymbol = dynamic_cast<SymbolVarnode*>(varnode)) {
				const auto it = m_symbolVarnodes.find(varnodeSymbol);
				if (it != m_symbolVarnodes.end()) {
					return it->second;
				}
			}
			else if (const auto varnodeConstant = dynamic_cast<ConstantVarnode*>(varnode)) {
				return varnodeConstant->m_value;
			}
			
			throw VmException("data not found");
		}

		void syncWith(const std::map<int, DataValue>& registers) {
			m_registers = registers;
			m_symbolVarnodes.clear();
		}
	};

	class VmMemoryContext
	{
		std::map<std::uintptr_t, DataValue> m_values;
		std::function<bool(std::uintptr_t, DataValue&)> m_addressSpaceCallaback;
	public:
		VmMemoryContext()
		{}

		void setValue(std::uintptr_t address, DataValue value) {
			m_values[address] = value;
		}

		DataValue getValue(std::uintptr_t address) {
			const auto it = m_values.find(address);
			if (it != m_values.end()) {
				return it->second;
			}
			DataValue value;
			if (m_addressSpaceCallaback(address, value))
				return value;
			throw VmException("data not found");
		}

		// after sync
		void clear() {
			m_values.clear();
		}

		void setAddressSpaceCallaback(const std::function<bool(std::uintptr_t, DataValue&)>& addressSpaceCallaback) {
			m_addressSpaceCallaback = addressSpaceCallaback;
		}
	};

	class VirtualMachine
	{
		VmExecutionContext* m_execCtx;
		VmMemoryContext* m_memCtx;
	public:
		VirtualMachine(VmExecutionContext* execCtx, VmMemoryContext* memCtx)
			: m_execCtx(execCtx), m_memCtx(memCtx)
		{}

		void execute(Instruction* instr) const {
			if(instr->m_id >= InstructionId::INT_ADD && instr->m_id <= InstructionId::INT_SREM) {
				executeArithmetic(instr);
			}
		}

	private:
		void executeArithmetic(Instruction* instr) const {
			DataValue result = 0;
			const auto op1 = m_execCtx->getValue(instr->m_input0);
			if(instr->m_id == InstructionId::INT_2COMP) {
				result = -op1;
			}
			else {
				const auto op2 = m_execCtx->getValue(instr->m_input1);
				switch (instr->m_id)
				{
				case InstructionId::INT_ADD: {
					result = op1 + op2;
					break;
				}
				case InstructionId::INT_SUB: {
					result = op1 - op2;
					break;
				}
				case InstructionId::INT_MULT: {
					result = op1 * op2;
					break;
				}
				case InstructionId::INT_DIV: {
					result = op1 / op2;
					break;
				}
				case InstructionId::INT_SDIV: {
					result = (int64_t&)op1 / (int64_t&)op2;
					break;
				}
				case InstructionId::INT_REM: {
					result = op1 % op2;
					break;
				}
				case InstructionId::INT_SREM: {
					result = (int64_t&)op1 % (int64_t&)op2;
					break;
				}
				case InstructionId::INT_CARRY: {
					// todo: add
					break;
				}
				case InstructionId::INT_SCARRY: {
					// todo: add
					break;
				}
				case InstructionId::INT_SBORROW: {
					// todo: add
					break;
				}
				}
			}
			m_execCtx->setValue(instr->m_output, result);
		}

		void executeLogical(Instruction* instr) const {
			DataValue result = 0;
			const auto op1 = m_execCtx->getValue(instr->m_input0);
			if (instr->m_id == InstructionId::INT_NEGATE) {
				result = ~op1;
			}
			else {
				const auto op2 = m_execCtx->getValue(instr->m_input1);
				switch (instr->m_id)
				{
				case InstructionId::INT_XOR: {
					result = op1 ^ op2;
					break;
				}
				case InstructionId::INT_AND: {
					result = op1 & op2;
					break;
				}
				case InstructionId::INT_OR: {
					result = op1 | op2;
					break;
				}
				case InstructionId::INT_LEFT: {
					result = op1 >> op2;
					break;
				}
				case InstructionId::INT_RIGHT: {
					result = op1 << op2;
					break;
				}
				case InstructionId::INT_SRIGHT: {
					result = op1 << op2;
					// todo: add
					break;
				}
				}
			}
			m_execCtx->setValue(instr->m_output, result);
		}

		void executeIntegerComp(Instruction* instr) const {
			DataValue result = 0;
			const auto op1 = m_execCtx->getValue(instr->m_input0);
			const auto op2 = m_execCtx->getValue(instr->m_input1);
			switch (instr->m_id)
			{
			case InstructionId::INT_EQUAL: {
				result = op1 == op2;
				break;
			}
			case InstructionId::INT_NOTEQUAL: {
				result = op1 != op2;
				break;
			}
			case InstructionId::INT_SLESS: {
				result = (int64_t&)op1 < (int64_t&)op2;
				break;
			}
			case InstructionId::INT_SLESSEQUAL: {
				result = (int64_t&)op1 <= (int64_t&)op2;
				break;
			}
			case InstructionId::INT_LESS: {
				result = op1 < op2;
				break;
			}
			case InstructionId::INT_LESSEQUAL: {
				result = op1 <= op2;
				break;
			}
			}
			m_execCtx->setValue(instr->m_output, result);
		}

		void executeBoolean(Instruction* instr) const {
			DataValue result = 0;
			const auto op1 = m_execCtx->getValue(instr->m_input0);
			if (instr->m_id == InstructionId::INT_NEGATE) {
				result = !op1;
			}
			else {
				const auto op2 = m_execCtx->getValue(instr->m_input1);
				switch (instr->m_id)
				{
				case InstructionId::BOOL_XOR: {
					result = op1 ^ op2;
					break;
				}
				case InstructionId::BOOL_AND: {
					result = op1 & op2;
					break;
				}
				case InstructionId::BOOL_OR: {
					result = op1 | op2;
					break;
				}
				}
			}
			m_execCtx->setValue(instr->m_output, result);
		}

		template<typename T>
		void executeFloatingPoint(Instruction* instr) const {
			T fresult = 0;
			if (instr->m_id == InstructionId::FLOAT_NAN) {
				fresult = std::numeric_limits<T>::quiet_NaN();
			}
			else {
				const auto op1 = m_execCtx->getValue(instr->m_input0);
				const auto fop1 = (T&)op1;
				
				if (instr->m_id >= InstructionId::FLOAT_NEG) {
					switch (instr->m_id)
					{
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
					}
				}
				else {
					const auto op2 = m_execCtx->getValue(instr->m_input1);
					const auto fop2 = (T&)op2;
					
					switch (instr->m_id)
					{
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
						fresult = fop1 / fop2;
						break;
					}
					}
				}
			}

			DataValue result = 0;
			(T&)result = fresult;
			m_execCtx->setValue(instr->m_output, result);
		}

		template<typename T>
		void executeFloatingPointComp(Instruction* instr) const {
			DataValue result = 0;
			const auto op1 = m_execCtx->getValue(instr->m_input0);
			const auto fop1 = (T&)op1;
			const auto op2 = m_execCtx->getValue(instr->m_input1);
			const auto fop2 = (T&)op2;
			switch (instr->m_id)
			{
			case InstructionId::FLOAT_EQUAL: {
				result = fop1 == fop2;
				break;
			}
			case InstructionId::FLOAT_NOTEQUAL: {
				result = fop1 != fop2;
				break;
			}
			case InstructionId::FLOAT_LESS: {
				result = fop1 < fop2;
				break;
			}
			case InstructionId::FLOAT_LESSEQUAL: {
				result = fop1 <= fop2;
				break;
			}
			}
			m_execCtx->setValue(instr->m_output, result);
		}

		template<typename T_in = float, typename T_out = float, typename T2_out = int>
		void executeFloatingPointConversion(Instruction* instr) const {
			DataValue result = 0;
			const auto op1 = m_execCtx->getValue(instr->m_input0);
			if (instr->m_id == InstructionId::INT2FLOAT) {
				(T_out&)result = (T_out)op1;
			}
			else {
				const auto fop1 = (T_in&)op1;
				if (instr->m_id == InstructionId::FLOAT2FLOAT) {
					(T_out&)result = (T_out)fop1;
				}
				else {
					switch (instr->m_id)
					{
					case InstructionId::FLOAT2INT: {
						(T2_out&)result = (T2_out)fop1;
						break;
					}
					case InstructionId::TRUNC: {
						(T2_out&)result = (T2_out)trunc(fop1);
						break;
					}
					case InstructionId::FLOAT_CEIL: {
						result = ceil(fop1);
						break;
					}
					case InstructionId::FLOAT_FLOOR: {
						result = floor(fop1);
						break;
					}
					case InstructionId::FLOAT_ROUND: {
						result = round(fop1);
						break;
					}
					}
				}
			}
			m_execCtx->setValue(instr->m_output, result);
		}

		void executeBranching(Instruction* instr) const {
			const auto op1 = m_execCtx->getValue(instr->m_input0);
			if(instr->m_id == InstructionId::CBRANCH) {
				const auto op2 = m_execCtx->getValue(instr->m_input1);
				if (!op2)
					return;
			}
			/*
			 * case InstructionId::BRANCH:
			 * case InstructionId::BRANCHIND:
			 * case InstructionId::CALL:
			 * case InstructionId::CALLIND:
			 * case InstructionId::RETURN:
			 */
			m_execCtx->m_nextInstrOffset = op1;
		}

		template<typename T_in = int, typename T_out = int64_t>
		void executeExtension(Instruction* instr) const {
			DataValue result = 0;
			const auto op1 = m_execCtx->getValue(instr->m_input0);
			if (instr->m_id == InstructionId::INT_SEXT) {
				(T_out&)result = (T_out)(T_in&)op1;
			} else {
				result = op1;
			}
			m_execCtx->setValue(instr->m_output, result);
		}
	};
};