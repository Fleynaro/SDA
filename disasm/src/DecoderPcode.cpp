#include "Disasm/DecoderPcode.h"

using namespace sda;
using namespace sda::disasm;

std::list<pcode::Instruction>&& DecoderPcode::getDecodedInstructions() {
    return std::move(m_decodedInstructions);
}

std::unique_ptr<DecoderPcode::Callbacks> DecoderPcode::setCallbacks(std::unique_ptr<Callbacks> callbacks) {
    auto oldCallbacks = std::move(m_callbacks);
    m_callbacks = std::move(callbacks);
    return oldCallbacks;
}

DecoderPcode::Callbacks* DecoderPcode::getCallbacks() const {
    return m_callbacks.get();
}