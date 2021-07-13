#include "DecPCodeAbstractDecoder.h"

using namespace CE::Decompiler;
using namespace CE::Decompiler::PCode;

void CE::Decompiler::PCode::AbstractDecoder::decode(void* addr, Offset offset, int maxSize) {
	m_addr = addr;
	m_curOrigInstr = nullptr;
	m_curOrderId = 0x0;
	m_maxSize = maxSize;
	clear();
	tryDecode(addr, offset);
}

void CE::Decompiler::PCode::AbstractDecoder::clear() {
	m_result.clear();
}

std::list<Instruction*>& CE::Decompiler::PCode::AbstractDecoder::getDecodedPCodeInstructions() {
	return m_result;
}

void CE::Decompiler::PCode::AbstractDecoder::deleteDecodedPCodeInstructions() {
	for (auto instr : getDecodedPCodeInstructions()) {
		delete instr;
	}
}

Instruction::OriginalInstruction* CE::Decompiler::PCode::AbstractDecoder::getOrigInstruction() const
{
	return m_curOrigInstr;
}

WarningContainer* CE::Decompiler::PCode::AbstractDecoder::getWarningContainer() {
	return m_warningContainer;
}
