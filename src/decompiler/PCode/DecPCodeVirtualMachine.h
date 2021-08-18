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

		const std::map<int, DataValue>& getRegisters() const;

		const std::map<SymbolVarnode*, DataValue>& getSymbolVarnodes() const;

		void setRegisterValue(const Register& reg, DataValue value);

		bool getRegisterValue(const Register& reg, DataValue& value) const;

		void setValue(Varnode* varnode, DataValue value);

		bool getValue(Varnode* varnode, DataValue& value) const;

		void syncWith(const std::map<int, DataValue>& registers);
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
		void clear();

		void setAddressSpaceCallaback(const std::function<bool(std::uintptr_t, DataValue&)>& addressSpaceCallaback);
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
		void executeFloatingPoint();

		template<typename T>
		void executeFloatingPointComp();

		template<typename T>
		void executeFloatingPointConversion();

		void executeFloatToFloat();

		void executeFloatToInt();

		void executeFloatTrunc();

		void executeBranching();

		void executeExtension();

		template<typename T_in = int, typename T_out = int64_t>
		void executeSignExtension();

		void executeTruncation();
		
		DataValue getValue(Varnode* varnode) const;
	};
};