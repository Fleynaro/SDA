#include "DecPCodeAbstractDecoder.h"

using namespace CE::Decompiler;
using namespace PCode;

void AbstractDecoder::decode(void* addr, Offset offset, int maxSize) {
	m_addr = addr;
	m_curOrigInstr = nullptr;
	m_curOrderId = 0x0;
	m_maxSize = maxSize;
	clear();
	tryDecode(addr, offset);
}

void AbstractDecoder::clear() {
	m_result.clear();
}

std::list<Instruction*>& AbstractDecoder::getDecodedPCodeInstructions() {
	return m_result;
}

void AbstractDecoder::deleteDecodedPCodeInstructions() {
	for (auto instr : getDecodedPCodeInstructions()) {
		delete instr;
	}
}

Instruction::OriginalInstruction* AbstractDecoder::getOrigInstruction() const
{
	return m_curOrigInstr;
}

WarningContainer* AbstractDecoder::getWarningContainer() {
	return m_warningContainer;
}
