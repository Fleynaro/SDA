#pragma once
#include <decompiler/DecStorage.h>
#include "DecExecContext.h"

namespace CE::Decompiler
{
	class AbstractPrimaryDecompiler;
};

namespace CE::Decompiler::PCode
{
	class InstructionInterpreter
	{
	public:
		InstructionInterpreter(AbstractPrimaryDecompiler* decompiler, DecBlock* block, ExecContext* ctx)
			: m_decompiler(decompiler), m_block(block), m_ctx(ctx)
		{}

		void execute(Instruction* instr);

		// create a parameter for some function call using the defined storage (rcx, rdx, [rsp - 0x8], ...)
		ExprTree::INode* buildParameterInfoExpr(ParameterInfo& paramInfo);

		// get expr. value from varnode (register/symbol/constant)
		ExprTree::INode* requestVarnode(Varnode* varnode) const;

		// make expression return boolean value: x -> x != 0
		ExprTree::AbstractCondition* toBoolean(ExprTree::INode* node);

		// create assignment line: memVar1 = read([memory location])
		ExprTree::SymbolLeaf* createMemSymbolLeaf(ExprTree::ReadValueNode* readValueNode, Instruction* instr = nullptr) const;
	private:
		AbstractPrimaryDecompiler* m_decompiler;
		DecBlock* m_block;
		ExecContext* m_ctx;
		Instruction* m_instr;
	};
};
