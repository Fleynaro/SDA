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
		bool m_throwException;
	public:
		DataValue m_nextInstrOffset = 0;
		
		VmExecutionContext(bool throwException = true)
			: m_throwException(throwException)
		{}

		const std::map<int, DataValue>& getRegisters() {
			return m_registers;
		}

		const std::map<SymbolVarnode*, DataValue>& getSymbolVarnodes() {
			return m_symbolVarnodes;
		}

		void setRegisterValue(const Register& reg, DataValue value) {
			m_registers[reg.getId()] = value;
		}

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

			if(m_throwException)
				throw VmException("data not found");
			return 0;
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
		bool m_throwException;
	public:
		VmMemoryContext(bool throwException = true)
			: m_throwException(throwException)
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
			if (m_throwException)
				throw VmException("data not found");
			return 0;
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
		DataValue m_op1 = 0;
		DataValue m_op2 = 0;
		DataValue m_result = 0;
		Instruction* m_instr = nullptr;
	public:
		VirtualMachine(VmExecutionContext* execCtx, VmMemoryContext* memCtx)
			: m_execCtx(execCtx), m_memCtx(memCtx)
		{}

		void execute(Instruction* instr) {
			m_instr = instr;
			m_op1 = 0;
			m_op2 = 0;
			m_result = 0;
			if (m_instr->m_input0) {
				m_op1 = m_execCtx->getValue(m_instr->m_input0);
			}
			if (m_instr->m_input1) {
				m_op2 = m_execCtx->getValue(m_instr->m_input1);
			}

			dispatch();

			if (m_instr->m_output) {
				m_execCtx->setValue(m_instr->m_output, m_result);
			}
		}

	private:
		void dispatch() {
			if (m_instr->m_id >= InstructionId::COPY && m_instr->m_id <= InstructionId::STORE) {
				executeDataMoving();
			}
			else if (m_instr->m_id >= InstructionId::INT_ADD && m_instr->m_id <= InstructionId::INT_SREM) {
				executeArithmetic();
			}
			else if (m_instr->m_id >= InstructionId::INT_NEGATE && m_instr->m_id <= InstructionId::INT_SRIGHT) {
				executeLogical();
			}
			else if (m_instr->m_id >= InstructionId::INT_EQUAL && m_instr->m_id <= InstructionId::INT_LESSEQUAL) {
				executeIntegerComp();
			}
			else if (m_instr->m_id >= InstructionId::BOOL_NEGATE && m_instr->m_id <= InstructionId::BOOL_OR) {
				executeBoolean();
			}
			else if (m_instr->m_id >= InstructionId::FLOAT_ADD && m_instr->m_id <= InstructionId::FLOAT_NAN) {
				if (m_instr->m_input0->getSize() == 0x4) {
					executeFloatingPoint<float>();
				}
				else {
					executeFloatingPoint<double>();
				}
			}
			else if (m_instr->m_id >= InstructionId::FLOAT_EQUAL && m_instr->m_id <= InstructionId::FLOAT_LESSEQUAL) {
				if (m_instr->m_input0->getSize() == 0x4) {
					executeFloatingPointComp<float>();
				}
				else {
					executeFloatingPointComp<double>();
				}
			}
			else if (m_instr->m_id >= InstructionId::INT2FLOAT && m_instr->m_id <= InstructionId::FLOAT_ROUND) {
				if(m_instr->m_id >= InstructionId::INT2FLOAT) {
					executeFloatToInt();
				} else if (m_instr->m_id >= InstructionId::FLOAT2FLOAT) {
					executeFloatToFloat();
				} else if (m_instr->m_id >= InstructionId::TRUNC) {
					executeFloatTrunc();
				}
				else {
					if (m_instr->m_input0->getSize() == 0x4) {
						executeFloatingPointConversion<float>();
					}
					else {
						executeFloatingPointConversion<double>();
					}
				}
			}
			else if (m_instr->m_id >= InstructionId::BRANCH && m_instr->m_id <= InstructionId::RETURN) {
				executeBranching();
			}
			else if (m_instr->m_id >= InstructionId::INT_ZEXT && m_instr->m_id <= InstructionId::INT_SEXT) {
				executeExtension();
			}
			else if (m_instr->m_id >= InstructionId::PIECE && m_instr->m_id <= InstructionId::SUBPIECE) {
				executeTruncation();
			}
		}
		
		void executeDataMoving() {
			switch (m_instr->m_id)
			{
			case InstructionId::COPY: {
				m_result = m_op1;
				break;
			}
			case InstructionId::LOAD: {
				m_result = m_memCtx->getValue(m_op1);
				break;
			}
			case InstructionId::STORE: {
				m_memCtx->setValue(m_op1, m_op2);
				break;
			}
			}
		}
		
		void executeArithmetic() {
			switch (m_instr->m_id)
			{
			case InstructionId::INT_2COMP: {
				m_result = -m_op1;
				break;
			}
			case InstructionId::INT_ADD: {
				m_result = m_op1 + m_op2;
				break;
			}
			case InstructionId::INT_SUB: {
				m_result = m_op1 - m_op2;
				break;
			}
			case InstructionId::INT_MULT: {
				m_result = m_op1 * m_op2;
				break;
			}
			case InstructionId::INT_DIV: {
				m_result = m_op1 / m_op2;
				break;
			}
			case InstructionId::INT_SDIV: {
				m_result = (int64_t&)m_op1 / (int64_t&)m_op2;
				break;
			}
			case InstructionId::INT_REM: {
				m_result = m_op1 % m_op2;
				break;
			}
			case InstructionId::INT_SREM: {
				m_result = (int64_t&)m_op1 % (int64_t&)m_op2;
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

		void executeLogical() {
			switch (m_instr->m_id)
			{
			case InstructionId::INT_NEGATE: {
				m_result = ~m_op1;
				break;
			}
			case InstructionId::INT_XOR: {
				m_result = m_op1 ^ m_op2;
				break;
			}
			case InstructionId::INT_AND: {
				m_result = m_op1 & m_op2;
				break;
			}
			case InstructionId::INT_OR: {
				m_result = m_op1 | m_op2;
				break;
			}
			case InstructionId::INT_LEFT: {
				m_result = m_op1 >> m_op2;
				break;
			}
			case InstructionId::INT_RIGHT: {
				m_result = m_op1 << m_op2;
				break;
			}
			case InstructionId::INT_SRIGHT: {
				m_result = m_op1 << m_op2;
				// todo: add
				break;
			}
			}
		}

		void executeIntegerComp() {
			switch (m_instr->m_id)
			{
			case InstructionId::INT_EQUAL: {
				m_result = m_op1 == m_op2;
				break;
			}
			case InstructionId::INT_NOTEQUAL: {
				m_result = m_op1 != m_op2;
				break;
			}
			case InstructionId::INT_SLESS: {
				m_result = (int64_t&)m_op1 < (int64_t&)m_op2;
				break;
			}
			case InstructionId::INT_SLESSEQUAL: {
				m_result = (int64_t&)m_op1 <= (int64_t&)m_op2;
				break;
			}
			case InstructionId::INT_LESS: {
				m_result = m_op1 < m_op2;
				break;
			}
			case InstructionId::INT_LESSEQUAL: {
				m_result = m_op1 <= m_op2;
				break;
			}
			}
			m_execCtx->setValue(m_instr->m_output, m_result);
		}

		void executeBoolean() {
			const auto op2 = m_execCtx->getValue(m_instr->m_input1);
			switch (m_instr->m_id)
			{
			case InstructionId::INT_NEGATE: {
				m_result = !m_op1;
				break;
			}
			case InstructionId::BOOL_XOR: {
				m_result = m_op1 ^ op2;
				break;
			}
			case InstructionId::BOOL_AND: {
				m_result = m_op1 & op2;
				break;
			}
			case InstructionId::BOOL_OR: {
				m_result = m_op1 | op2;
				break;
			}
			}
		}

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
				fresult = fop1 / fop2;
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

		void executeFloatToFloat() {
			const auto sizeInput = m_instr->m_input0->getSize();
			const auto sizeOut = m_instr->m_output->getSize();
			if (sizeInput == 0x4) {
				if (sizeOut == 0x8)
					(double&)m_result = (double)(float&)m_op1;
			}
		}

		void executeFloatToInt() {
			const auto sizeInput = m_instr->m_input0->getSize();
			if (sizeInput == 0x4)
				(int64_t&)m_result = (int64_t)(float&)m_op1;
			else if(sizeInput == 0x8)
				(int64_t&)m_result = (int64_t)(double&)m_op1;
		}

		void executeFloatTrunc() {
			const auto sizeInput = m_instr->m_input0->getSize();
			if (sizeInput == 0x4)
				(int64_t&)m_result = (int64_t)trunc((float&)m_op1);
			else if (sizeInput == 0x8)
				(int64_t&)m_result = (int64_t)trunc((double&)m_op1);
		}

		void executeBranching() {
			if(m_instr->m_id == InstructionId::CBRANCH) {
				if (!m_op2)
					return;
			}
			/*
			 * case InstructionId::BRANCH:
			 * case InstructionId::BRANCHIND:
			 * case InstructionId::CALL:
			 * case InstructionId::CALLIND:
			 * case InstructionId::RETURN:
			 */
			m_execCtx->m_nextInstrOffset = m_op1;
		}

		void executeExtension() {
			if (m_instr->m_id == InstructionId::INT_SEXT) {
				const auto size1 = m_instr->m_input0->getSize();
				const auto size2 = m_instr->m_input1->getSize();
				if (size1 == 0x1) {
					if (size2 == 0x2)
						executeSignExtension<int8_t, int16_t>();
					else if (size2 == 0x4)
						executeSignExtension<int8_t, int32_t>();
					else
						executeSignExtension<int8_t, int64_t>();
				}
				else if (size1 == 0x2) {
					if (size2 == 0x4)
						executeSignExtension<int16_t, int32_t>();
					else if (size2 == 0x8)
						executeSignExtension<int16_t, int64_t>();
				}
				else if (size1 == 0x4) {
					executeSignExtension<int32_t, int64_t>();
				}
			}
			else {
				// INT_ZEXT
				m_result = m_op1;
			}
		}

		template<typename T_in = int, typename T_out = int64_t>
		void executeSignExtension() {
			(T_out&)m_result = (T_out)(T_in&)m_op1;
		}

		void executeTruncation() {
			switch (m_instr->m_id)
			{
			case InstructionId::PIECE: {
				m_result = m_op1 | m_op2 << m_instr->m_input0->getSize();
				break;
			}
			case InstructionId::SUBPIECE: {
				m_result = m_op1 >> m_op2;
				break;
			}
			}
		}
	};
};