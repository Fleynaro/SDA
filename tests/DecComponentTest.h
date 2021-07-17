#pragma once
#include "AbstractTest.h"

using namespace CE::Decompiler;
using namespace CE::Symbol;
using namespace CE::DataType;

class ProgramDecCompFixture : public ProgramDecFixture {
public:
	PCode::VirtualMachineContext m_vmCtx;
	InstructionPool m_instrPool;
	PCode::Instruction::OriginalInstruction* m_origInstr;

	ProgramDecCompFixture() {
		createProject("test");
		m_origInstr = m_instrPool.createOrigInstruction(0, 1);
	}

	int m_instrOrderId = 0;
	PCode::Instruction* createInstr(InstructionId id, Varnode* input0, Varnode* input1, Varnode* output) {
		return m_instrPool.createInstruction(id, input0, input1, output, m_origInstr, m_instrOrderId++);
	}

	// decode {bytes} into pcode instructions
	std::list<Instruction*> decode(std::vector<byte> bytes) {
		std::list<Instruction*> decodedInstructions;
		WarningContainer warningContainer;
		PCode::DecoderX86 decoder(&m_registerFactoryX86, &m_instrPool, &warningContainer);
		int offset = 0;
		while (offset < bytes.size()) {
			decoder.decode(bytes.data() + offset, offset, (int)bytes.size());
			if (!decoder.getOrigInstruction())
				break;
			decodedInstructions.insert(decodedInstructions.end(), decoder.getDecodedPCodeInstructions().begin(), decoder.getDecodedPCodeInstructions().end());
			offset += decoder.getOrigInstruction()->m_length;
		}
		return decodedInstructions;
	}

	// show all pcode instructions with original asm instructions
	void showInstructions(const std::list<Instruction*>& instructions) {
		for(auto instr : instructions) {
			out(InstructionTextGenerator().getText(instr).c_str());
		}
	}

	// execute pcode on the virtual machine
	std::map<PCode::Instruction*, DataValue> executeAndCalcConstValue(std::list<Instruction*> decodedInstructions) {
		std::map<PCode::Instruction*, DataValue> constValues;
		PCode::ConstValueCalculating constValueCalculating(decodedInstructions, &m_vmCtx, &m_registerFactoryX86);
		constValueCalculating.start(constValues);
		return constValues;
	}

	// show const values calculated by the virtual machine
	void showConstValues(std::map<PCode::Instruction*, DataValue> constValues) {
		for (const auto& [instr, dataValue] : constValues) {
			printf("%s -> %64i", InstructionTextGenerator().getText(instr).c_str(), dataValue);
		}
	}

	// need to optimize some expr. to one constant value
	void replaceSymbolWithExpr(INode* node, CE::Decompiler::Symbol::Symbol* symbol, INode* newNode) {
		node->iterateChildNodes([&](INode* childNode) {
			replaceSymbolWithExpr(childNode, symbol, newNode);
			});
		if (auto symbolLeaf = dynamic_cast<SymbolLeaf*>(node)) {
			if (symbolLeaf->m_symbol == symbol) {
				node->replaceWith(newNode);
				delete node;
			}
		}
	}

	// optimize expr.
	void optimize(TopNode* topNode) {
		Optimization::ExprOptimization exprOptimization(topNode);
		exprOptimization.start();
	}
};