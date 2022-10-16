#include "SDA/Core/Platform/InstructionDecoder.h"

using namespace sda;

const Instruction* InstructionDecoder::getDecodedInstruction() const {
    return &m_decodedInstruction;
}