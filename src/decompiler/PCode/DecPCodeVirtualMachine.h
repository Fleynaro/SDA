#pragma once
#include "DecPCode.h"

namespace CE::Decompiler::PCode
{
	class VirtualMachineContext
	{
		struct RegisterStorage {
			DataValue m_data[4] = { 0x0, 0x0, 0x0, 0x0 };
		};

		std::map<RegisterId, RegisterStorage> m_registers;
		std::map<SymbolVarnode*, DataValue> m_symbolVarnodes;
	public:
		VirtualMachineContext()
		{}

		void setConstantValue(const Register& reg, DataValue value) {
			RegisterStorage regStorage;
			auto& dataCell = regStorage.m_data[reg.getIndex()];
			dataCell = dataCell & ~GetValueRangeMaskWithException(reg).getValue() | value;
			m_registers[reg.getId()] = regStorage;
		}

		void setConstantValue(Varnode* varnode, DataValue value) {
			if (auto varnodeRegister = dynamic_cast<PCode::RegisterVarnode*>(varnode)) {
				setConstantValue(varnodeRegister->m_register, value);
			}
			else if (auto varnodeSymbol = dynamic_cast<PCode::SymbolVarnode*>(varnode)) {
				m_symbolVarnodes[varnodeSymbol] = value;
			}
		}

		bool tryGetConstantValue(Varnode* varnode, DataValue& value) {
			if (auto varnodeRegister = dynamic_cast<PCode::RegisterVarnode*>(varnode)) {
				auto& reg = varnodeRegister->m_register;
				auto it = m_registers.find(reg.getId());
				if (it != m_registers.end()) {
					value = it->second.m_data[reg.getIndex()] >> reg.m_valueRangeMask.getOffset();
					return true;
				}
			}
			else if(auto varnodeSymbol = dynamic_cast<PCode::SymbolVarnode*>(varnode)) {
				auto it = m_symbolVarnodes.find(varnodeSymbol);
				if (it != m_symbolVarnodes.end()) {
					value = it->second;
					return true;
				}
			}
			else if (auto varnodeConstant = dynamic_cast<PCode::ConstantVarnode*>(varnode)) {
				value = varnodeConstant->m_value;
				return true;
			}
			return false;
		}

		void clear(Varnode* varnode) {
			if (auto varnodeRegister = dynamic_cast<PCode::RegisterVarnode*>(varnode)) {
				m_registers.erase(varnodeRegister->m_register.getId());
			}
			else if (auto varnodeSymbol = dynamic_cast<PCode::SymbolVarnode*>(varnode)) {
				m_symbolVarnodes.erase(varnodeSymbol);
			}
		}
	};

	class VirtualMachine
	{
		VirtualMachineContext* m_virtualMachineCtx;
	public:
		VirtualMachine(VirtualMachineContext* virtualMachineContext)
			: m_virtualMachineCtx(virtualMachineContext)
		{}

		void execute(Instruction* instr) const
		{
			switch (instr->m_id)
			{
			case InstructionId::INT_ADD:
			case InstructionId::INT_SUB:
			case InstructionId::INT_MULT:
				DataValue op1;
				DataValue op2;
				if (m_virtualMachineCtx->tryGetConstantValue(instr->m_input0, op1)) {
					if (m_virtualMachineCtx->tryGetConstantValue(instr->m_input1, op2)) {
						DataValue result;
						switch (instr->m_id)
						{
						case InstructionId::INT_ADD:
							result = op1 + op2;
							break;
						case InstructionId::INT_SUB:
							result = op1 - op2;
							break;
						case InstructionId::INT_MULT:
							result = op1 * op2;
							break;
						}
						m_virtualMachineCtx->setConstantValue(instr->m_output, result);
						return;
					}
				}
				break;
			}

			if (instr->m_output) {
				m_virtualMachineCtx->clear(instr->m_output);
			}
		}
	};
};