#pragma once
#include "PCode/DecPCode.h"
#include <utilities/HashSerialization.h>
#include <utilities/Helper.h>

namespace CE::Decompiler {
	class DecompiledCodeGraph;
	namespace ExprTree {
		struct NodeCloneContext;
	};
};

namespace CE::Decompiler::Symbol
{
	// symbol of decompiler level that has some concrete value (low-level, before symbolization)
	class Symbol
	{
	public:
		int m_size;
		int m_parentsCount = 0;
		
		Symbol(int size)
			: m_size(size)
		{}

		virtual ~Symbol() {}

		int getSize() const
		{
			return m_size;
		}

		virtual HS getHash() = 0;

		Symbol* clone(ExprTree::NodeCloneContext* ctx);

	protected:
		virtual Symbol* cloneSymbol() = 0;
	};
	
	// variable that is associated with the register and dont contain direct expr. value
	class RegisterVariable : public Symbol
	{
	public:
		PCode::Register m_register;
		
		RegisterVariable(const PCode::Register& reg)
			: Symbol(reg.getSize()), m_register(reg)
		{}

		HS getHash() override {
			return HS()
				<< m_register.getId()
				<< m_register.m_valueRangeMask.getSize()
				<< m_register.m_valueRangeMask.getOffset();
		}

	protected:
		Symbol* cloneSymbol() override {
			return new RegisterVariable(m_register);
		}
	};

	// variable on high-level
	class AbstractVariable : public Symbol, public PCode::IRelatedToInstruction {
	public:
		AbstractVariable(int size)
			: Symbol(size)
		{}

		HS getHash() override {
			HS hs(0);
			for (auto instr : getInstructionsRelatedTo())
				hs = hs + (HS() << instr->getOffset());
			return hs;
		}

		int getId() {
			return getHash().getHashValue() % 10000;
		}
	};

	// variable that is used because impossible to use expr. value directly from register (there are if/else/for)
	class LocalVariable : public AbstractVariable
	{
	public:
		PCode::Register m_register;
		std::list<PCode::Instruction*> m_instructionsRelatedTo;
		bool m_isTemp;

		LocalVariable(int size, const PCode::Register& reg, bool isTemp = false)
			: AbstractVariable(size), m_register(reg), m_isTemp(isTemp)
		{}

		void setSize(int size) {
			m_size = size;
		}

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override {
			return m_instructionsRelatedTo;
		}

	protected:
		Symbol* cloneSymbol() override {
			auto localVar = new LocalVariable(getSize(), m_register);
			localVar->m_instructionsRelatedTo = m_instructionsRelatedTo;
			return localVar;
		}
	};

	// variable that contains value from some memory location
	class MemoryVariable : public AbstractVariable
	{
	public:
		PCode::Instruction* m_instr;
		
		MemoryVariable(PCode::Instruction* instr, int size)
			: m_instr(instr), AbstractVariable(size)
		{}

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override {
			if(!m_instr)
				return {};
			return { m_instr };
		}

	protected:
		Symbol* cloneSymbol() override {
			return new MemoryVariable(m_instr, getSize());
		}
	};

	// variable that has a result of some function call
	class FunctionResultVar : public AbstractVariable
	{
	public:
		PCode::Register m_register;
		PCode::Instruction* m_instr;

		FunctionResultVar(PCode::Instruction* instr, int size, const PCode::Register& reg)
			: AbstractVariable(size), m_instr(instr), m_register(reg)
		{}

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override {
			if (!m_instr)
				return {};
			return { m_instr };
		}

	protected:
		Symbol* cloneSymbol() override {
			return new FunctionResultVar(m_instr, getSize(), m_register);
		}
	};
};