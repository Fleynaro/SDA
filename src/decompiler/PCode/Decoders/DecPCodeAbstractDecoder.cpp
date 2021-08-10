#include "DecPCodeAbstractDecoder.h"

using namespace CE::Decompiler;
using namespace PCode;

void AbstractDecoder::decode(Offset offset, const std::vector<uint8_t>& data) {
	m_curOffset = offset;
	m_curOrigInstr = nullptr;
	m_curOrderId = 0x0;
	clear();
	tryDecode(data);
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
