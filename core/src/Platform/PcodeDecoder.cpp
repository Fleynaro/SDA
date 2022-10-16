#include "SDA/Core/Platform/PcodeDecoder.h"

using namespace sda;

std::list<pcode::Instruction>&& PcodeDecoder::getDecodedInstructions() {
    return std::move(m_decodedInstructions);
}

void PcodeDecoder::setCallbacks(std::shared_ptr<Callbacks> callbacks) {
    m_callbacks = callbacks;
}

std::shared_ptr<PcodeDecoder::Callbacks> PcodeDecoder::getCallbacks() const {
    return m_callbacks;
}