#pragma once
#include "DecPCodeVirtualMachine.h"
#include "DecRegisterFactory.h"

namespace CE::Decompiler::PCode
{
	// Allow to calculate constant expression (e.g. for function addresses)
	class ConstValueCalculating
	{
		std::list<Instruction*> m_instructions;
		AbstractRegisterFactory* m_registerFactory;
	public:
		VmExecutionContext m_execCtx;
		VmMemoryContext m_memCtx;
		
		ConstValueCalculating(const std::list<Instruction*>& instructions, AbstractRegisterFactory* registerFactory)
			: m_instructions(instructions), m_registerFactory(registerFactory)
		{}

		void start(std::map<Instruction*, DataValue>& constValues) {
			VirtualMachine vm(&m_execCtx, &m_memCtx, false);
			//SP, IP registers to zero
			const auto rip = m_registerFactory->createInstructionPointerRegister();
			m_execCtx.setRegisterValue(m_registerFactory->createStackPointerRegister(), 0);
			for (auto instr : m_instructions) {
				m_execCtx.setRegisterValue(rip, instr->m_origInstruction->m_offset);
				vm.execute(instr);
				if (Instruction::IsAnyJmup(instr->m_id)) {
					if (!dynamic_cast<ConstantVarnode*>(instr->m_input0)) {
						DataValue value;
						if(m_execCtx.getValue(instr->m_input0, value))
							constValues[instr] = value;
					}
				}
				/*else if (instr->m_id == InstructionId::LOAD) {
					const auto value = m_execCtx.getValue(instr->m_input0);
					constValues[instr] = value;
				}*/
			}
		}
	};
};