#include "Disasm/DecoderPcode.h"

using namespace sda;
using namespace sda::disasm;

std::list<pcode::Instruction>&& DecoderPcode::getDecodedInstructions() {
    return std::move(m_decodedInstructions);
}

void DecoderPcode::setCallbacks(std::shared_ptr<Callbacks> callbacks) {
    m_callbacks = callbacks;
}

std::shared_ptr<DecoderPcode::Callbacks> DecoderPcode::getCallbacks() const {
    return m_callbacks;
}