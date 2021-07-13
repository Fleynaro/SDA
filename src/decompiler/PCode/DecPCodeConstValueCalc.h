#pragma once
#include "DecPCodeVirtualMachine.h"
#include "DecRegisterFactory.h"

namespace CE::Decompiler::PCode
{
	// Allow to calculate constant expression (e.g. for function addresses)
	class ConstValueCalculating
	{
		std::list<Instruction*> m_instructions;
		VirtualMachineContext* m_vmCtx;
		AbstractRegisterFactory* m_registerFactory;
	public:
		ConstValueCalculating(const std::list<Instruction*>& instructions, VirtualMachineContext* vmCtx, AbstractRegisterFactory* registerFactory)
			: m_instructions(instructions), m_vmCtx(vmCtx), m_registerFactory(registerFactory)
		{}

		void start(std::map<Instruction*, DataValue>& constValues) {
			const VirtualMachine vm(m_vmCtx);
			//SP, IP registers to zero
			m_vmCtx->setConstantValue(m_registerFactory->createInstructionPointerRegister(), 0);
			m_vmCtx->setConstantValue(m_registerFactory->createStackPointerRegister(), 0);
			for (auto instr : m_instructions) {
				vm.execute(instr);
				DataValue value;
				if (Instruction::IsAnyJmup(instr->m_id)) {
					if (!dynamic_cast<ConstantVarnode*>(instr->m_input0)) {
						if (m_vmCtx->tryGetConstantValue(instr->m_input0, value)) {
							constValues[instr] = value;
						}
					}
				}
				else if (instr->m_id == InstructionId::LOAD) {
					if (m_vmCtx->tryGetConstantValue(instr->m_input0, value)) {
						constValues[instr] = value;
					}
				}
			}
		}

	private:
	};
};