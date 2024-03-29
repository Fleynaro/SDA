#include "DecPCodeVirtualMachine.h"

const std::map<int, CE::Decompiler::PCode::DataValue>& CE::Decompiler::PCode::VmExecutionContext::getRegisters() const {
	return m_registers;
}

const std::map<CE::Decompiler::PCode::SymbolVarnode*, CE::Decompiler::PCode::DataValue>& CE::Decompiler::PCode::
VmExecutionContext::getSymbolVarnodes() const {
	return m_symbolVarnodes;
}

void CE::Decompiler::PCode::VmExecutionContext::setRegisterValue(const Register& reg, DataValue value) {
	m_registers[reg.getId()] = value;
}

bool CE::Decompiler::PCode::VmExecutionContext::getRegisterValue(const Register& reg, DataValue& value) const {
	const auto it = m_registers.find(reg.getId());
	if (it == m_registers.end())
		return false;
	value = it->second;
	return true;
}

void CE::Decompiler::PCode::VmExecutionContext::setValue(Varnode* varnode, DataValue value) {
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

bool CE::Decompiler::PCode::VmExecutionContext::getValue(Varnode* varnode, DataValue& value) const {
	if (const auto varnodeRegister = dynamic_cast<RegisterVarnode*>(varnode)) {
		const auto& reg = varnodeRegister->m_register;
		const auto it = m_registers.find(reg.getId());
		if (it != m_registers.end()) {
			const auto regValue = it->second & reg.m_valueRangeMask.getValue();
			value = regValue >> reg.m_valueRangeMask.getOffset();
			return true;
		}
	}
	else if (const auto varnodeSymbol = dynamic_cast<SymbolVarnode*>(varnode)) {
		const auto it = m_symbolVarnodes.find(varnodeSymbol);
		if (it != m_symbolVarnodes.end()) {
			value = it->second;
			return true;
		}
	}
	else if (const auto varnodeConstant = dynamic_cast<ConstantVarnode*>(varnode)) {
		value = varnodeConstant->m_value;
		return true;
	}
	return false;
}

void CE::Decompiler::PCode::VmExecutionContext::syncWith(const std::map<int, DataValue>& registers) {
	m_registers = registers;
	m_symbolVarnodes.clear();
}

void CE::Decompiler::PCode::VmMemoryContext::setValue(std::uintptr_t address, DataValue value) {
	m_values[address] = value;
}

bool CE::Decompiler::PCode::VmMemoryContext::getValue(std::uintptr_t address, DataValue& value) {
	const auto it = m_values.find(address);
	if (it != m_values.end()) {
		value = it->second;
		return true;
	}
	if (m_hasCallback) {
		return m_addressSpaceCallaback(address, value);
	}
	return false;
}

void CE::Decompiler::PCode::VmMemoryContext::clear() {
	m_values.clear();
}

void CE::Decompiler::PCode::VmMemoryContext::setAddressSpaceCallaback(
	const std::function<bool(std::uintptr_t, DataValue&)>& addressSpaceCallaback) {
	m_addressSpaceCallaback = addressSpaceCallaback;
	m_hasCallback = true;
}

void CE::Decompiler::PCode::VirtualMachine::execute(Instruction* instr) {
	m_instr = instr;
	m_op1 = 0;
	m_op2 = 0;
	m_result = 0;
	if (m_instr->m_input0) {
		m_op1 = getValue(m_instr->m_input0);
	}
	if (m_instr->m_input1) {
		m_op2 = getValue(m_instr->m_input1);
	}

	dispatch();

	if (m_instr->m_output) {
		m_execCtx->setValue(m_instr->m_output, m_result);
	}
}

void CE::Decompiler::PCode::VirtualMachine::dispatch() {
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
	else if (m_instr->m_id >= InstructionId::FLOAT_ADD && m_instr->m_id <= InstructionId::FLOAT_SQRT) {
		if (m_instr->m_input0->getSize() == 0x4) {
			executeFloatingPoint<float>();
		}
		else {
			executeFloatingPoint<double>();
		}
	}
	else if (m_instr->m_id >= InstructionId::FLOAT_NAN && m_instr->m_id <= InstructionId::FLOAT_LESSEQUAL) {
		if (m_instr->m_input0->getSize() == 0x4) {
			executeFloatingPointComp<float>();
		}
		else {
			executeFloatingPointComp<double>();
		}
	}
	else if (m_instr->m_id >= InstructionId::INT2FLOAT && m_instr->m_id <= InstructionId::FLOAT_ROUND) {
		if (m_instr->m_id >= InstructionId::INT2FLOAT) {
			executeFloatToInt();
		}
		else if (m_instr->m_id >= InstructionId::FLOAT2FLOAT) {
			executeFloatToFloat();
		}
		else if (m_instr->m_id >= InstructionId::TRUNC) {
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

void CE::Decompiler::PCode::VirtualMachine::executeDataMoving() {
	switch (m_instr->m_id) {
	case InstructionId::COPY: {
		m_result = m_op1;
		break;
	}
	case InstructionId::LOAD: {
		if (!m_memCtx->getValue(m_op1, m_result)) {
			if (m_throwException)
				throw VmException("data not found");
		}
		break;
	}
	case InstructionId::STORE: {
		m_memCtx->setValue(m_op1, m_op2);
		break;
	}
	}
}

template<typename T>
T IntegerSDiv(T a, T b) {
	return a / b;
}

template<typename T>
T IntegerSRem(T a, T b) {
	return a % b;
}

// checks for signed addition overflow
template<typename T>
bool IntegerSCarry(T a, T b) {
	if (a >= 0) {
		if (b > std::numeric_limits<T>::max() - a)
			return true;
	}
	else {
		if (b < std::numeric_limits<T>::min() - a)
			return true;
	}
	return false;
}

// checks for signed subtraction overflow
template<typename T>
bool IntegerSBorrow(T a, T b) {
	const auto result = a - b;
	return (result < a) != (b > 0);
}

template<typename T>
bool IntegerSLess(T a, T b) {
	return a < b;
}

template<typename T>
bool IntegerSLessEqual(T a, T b) {
	return a <= b;
}

void CE::Decompiler::PCode::VirtualMachine::executeArithmetic() {
	switch (m_instr->m_id) {
	case InstructionId::INT_2COMP: {
		m_result = -(int64_t&)m_op1;
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
		if(m_op2 != 0)
			m_result = m_op1 / m_op2;
		// todo: throw exception
		break;
	}
	case InstructionId::INT_SDIV: {
		if (m_op2 != 0) {
			const auto size = m_instr->m_input0->getSize();
			switch (size)
			{
			case 1:
				(int8_t&)m_result = IntegerSDiv((int8_t&)m_op1, (int8_t&)m_op2);
				break;
			case 2:
				(int16_t&)m_result = IntegerSDiv((int16_t&)m_op1, (int16_t&)m_op2);
				break;
			case 4:
				(int32_t&)m_result = IntegerSDiv((int32_t&)m_op1, (int32_t&)m_op2);
				break;
			case 8:
				(int64_t&)m_result = IntegerSDiv((int64_t&)m_op1, (int64_t&)m_op2);
				break;
			}
		}
		// todo: throw exception
		break;
	}
	case InstructionId::INT_REM: {
		m_result = m_op1 % m_op2;
		break;
	}
	case InstructionId::INT_SREM: {
		if (m_op2 != 0) {
			const auto size = m_instr->m_input0->getSize();
			switch (size)
			{
			case 1:
				(int8_t&)m_result = IntegerSRem((int8_t&)m_op1, (int8_t&)m_op2);
				break;
			case 2:
				(int16_t&)m_result = IntegerSRem((int16_t&)m_op1, (int16_t&)m_op2);
				break;
			case 4:
				(int32_t&)m_result = IntegerSRem((int32_t&)m_op1, (int32_t&)m_op2);
				break;
			case 8:
				(int64_t&)m_result = IntegerSRem((int64_t&)m_op1, (int64_t&)m_op2);
				break;
			}
		}
		// todo: throw exception
		break;
	}
	case InstructionId::INT_CARRY: {
		// checks for unsigned addition overflow
		const auto add = m_op1 + m_op2;
		m_result = add < m_op1;
		break;
	}
	case InstructionId::INT_SCARRY: {
		// checks for signed addition overflow
		const auto size = m_instr->m_input0->getSize();
		switch(size)
		{
		case 1:
			m_result = IntegerSCarry((int8_t&)m_op1, (int8_t&)m_op2);
			break;
		case 2:
			m_result = IntegerSCarry((int16_t&)m_op1, (int16_t&)m_op2);
			break;
		case 4:
			m_result = IntegerSCarry((int32_t&)m_op1, (int32_t&)m_op2);
			break;
		case 8:
			m_result = IntegerSCarry((int64_t&)m_op1, (int64_t&)m_op2);
			break;
		}
		break;
	}
	case InstructionId::INT_SBORROW: {
		// checks for signed subtraction overflow
		const auto size = m_instr->m_input0->getSize();
		switch (size)
		{
		case 1:
			m_result = IntegerSBorrow((int8_t&)m_op1, (int8_t&)m_op2);
			break;
		case 2:
			m_result = IntegerSBorrow((int16_t&)m_op1, (int16_t&)m_op2);
			break;
		case 4:
			m_result = IntegerSBorrow((int32_t&)m_op1, (int32_t&)m_op2);
			break;
		case 8:
			m_result = IntegerSBorrow((int64_t&)m_op1, (int64_t&)m_op2);
			break;
		}
		break;
	}
	}
}

void CE::Decompiler::PCode::VirtualMachine::executeLogical() {
	switch (m_instr->m_id) {
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

void CE::Decompiler::PCode::VirtualMachine::executeIntegerComp() {
	switch (m_instr->m_id) {
	case InstructionId::INT_EQUAL: {
		m_result = m_op1 == m_op2;
		break;
	}
	case InstructionId::INT_NOTEQUAL: {
		m_result = m_op1 != m_op2;
		break;
	}
	case InstructionId::INT_SLESS: {
		const auto size = m_instr->m_input0->getSize();
		switch (size)
		{
		case 1:
			(int8_t&)m_result = IntegerSLess((int8_t&)m_op1, (int8_t&)m_op2);
			break;
		case 2:
			(int16_t&)m_result = IntegerSLess((int16_t&)m_op1, (int16_t&)m_op2);
			break;
		case 4:
			(int32_t&)m_result = IntegerSLess((int32_t&)m_op1, (int32_t&)m_op2);
			break;
		case 8:
			(int64_t&)m_result = IntegerSLess((int64_t&)m_op1, (int64_t&)m_op2);
			break;
		}
		break;
	}
	case InstructionId::INT_SLESSEQUAL: {
		const auto size = m_instr->m_input0->getSize();
		switch (size)
		{
		case 1:
			(int8_t&)m_result = IntegerSLessEqual((int8_t&)m_op1, (int8_t&)m_op2);
			break;
		case 2:
			(int16_t&)m_result = IntegerSLessEqual((int16_t&)m_op1, (int16_t&)m_op2);
			break;
		case 4:
			(int32_t&)m_result = IntegerSLessEqual((int32_t&)m_op1, (int32_t&)m_op2);
			break;
		case 8:
			(int64_t&)m_result = IntegerSLessEqual((int64_t&)m_op1, (int64_t&)m_op2);
			break;
		}
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

void CE::Decompiler::PCode::VirtualMachine::executeBoolean() {
	switch (m_instr->m_id) {
	case InstructionId::INT_NEGATE: {
		m_result = !m_op1;
		break;
	}
	case InstructionId::BOOL_XOR: {
		m_result = m_op1 ^ m_op2;
		break;
	}
	case InstructionId::BOOL_AND: {
		m_result = m_op1 & m_op2;
		break;
	}
	case InstructionId::BOOL_OR: {
		m_result = m_op1 | m_op2;
		break;
	}
	}
}

template <typename T>
void CE::Decompiler::PCode::VirtualMachine::executeFloatingPoint() {
	T fresult = 0;
	const auto fop1 = (T&)m_op1;
	const auto fop2 = (T&)m_op2;

	switch (m_instr->m_id) {
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
		if (fop2 != 0)
			fresult = fop1 / fop2;
		// todo: throw exception
		break;
	}
	}

	(T&)m_result = fresult;
}

template <typename T>
void CE::Decompiler::PCode::VirtualMachine::executeFloatingPointComp() {
	const auto fop1 = (T&)m_op1;
	const auto fop2 = (T&)m_op2;
	switch (m_instr->m_id) {
	case InstructionId::FLOAT_NAN: {
		m_result = isnan(fop1);
		break;
	}
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

template <typename T>
void CE::Decompiler::PCode::VirtualMachine::executeFloatingPointConversion() {
	T fresult = 0;
	const auto fop1 = (T&)m_op1;
	const auto fop2 = (T&)m_op2;
	switch (m_instr->m_id) {
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

template <typename T_in, typename T_out>
void CE::Decompiler::PCode::VirtualMachine::executeSignExtension() {
	(T_out&)m_result = (T_out)(T_in&)m_op1;
}

void CE::Decompiler::PCode::VirtualMachine::executeFloatToFloat() {
	const auto sizeIn = m_instr->m_input0->getSize();
	const auto sizeOut = m_instr->m_output->getSize();
	if (sizeIn == 0x4) {
		if (sizeOut == 0x8)
			(double&)m_result = (double)(float&)m_op1;
	}
}

void CE::Decompiler::PCode::VirtualMachine::executeFloatToInt() {
	const auto sizeInput = m_instr->m_input0->getSize();
	if (sizeInput == 0x4)
		(int64_t&)m_result = (int64_t)(float&)m_op1;
	else if (sizeInput == 0x8)
		(int64_t&)m_result = (int64_t)(double&)m_op1;
}

void CE::Decompiler::PCode::VirtualMachine::executeFloatTrunc() {
	const auto sizeInput = m_instr->m_input0->getSize();
	if (sizeInput == 0x4)
		(int64_t&)m_result = (int64_t)trunc((float&)m_op1);
	else if (sizeInput == 0x8)
		(int64_t&)m_result = (int64_t)trunc((double&)m_op1);
}

void CE::Decompiler::PCode::VirtualMachine::executeBranching() {
	if (m_instr->m_id == InstructionId::CBRANCH) {
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
	m_execCtx->m_nextInstrAddr = m_op1;
}

void CE::Decompiler::PCode::VirtualMachine::executeExtension() {
	if (m_instr->m_id == InstructionId::INT_SEXT) {
		const auto sizeIn = m_instr->m_input0->getSize();
		const auto sizeOut = m_instr->m_output->getSize();
		if (sizeIn == 0x1) {
			if (sizeOut == 0x2)
				executeSignExtension<int8_t, int16_t>();
			else if (sizeOut == 0x4)
				executeSignExtension<int8_t, int32_t>();
			else
				executeSignExtension<int8_t, int64_t>();
		}
		else if (sizeIn == 0x2) {
			if (sizeOut == 0x4)
				executeSignExtension<int16_t, int32_t>();
			else if (sizeOut == 0x8)
				executeSignExtension<int16_t, int64_t>();
		}
		else if (sizeIn == 0x4) {
			executeSignExtension<int32_t, int64_t>();
		}
	}
	else {
		// INT_ZEXT
		m_result = m_op1;
	}
}

void CE::Decompiler::PCode::VirtualMachine::executeTruncation() {
	switch (m_instr->m_id) {
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

CE::Decompiler::PCode::DataValue CE::Decompiler::PCode::VirtualMachine::getValue(Varnode* varnode) const {
	DataValue value = 0;
	if (!m_execCtx->getValue(varnode, value)) {
		if (m_throwException) {
			throw VmException("data not found");
		}
		// value by default
		m_execCtx->setValue(varnode, value);
	}
	return value;
}
