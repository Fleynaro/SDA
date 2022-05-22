#include "Disasm/InstructionRender.h"

using namespace sda::disasm;

void Instruction::Render::render(const Instruction* instruction) const {
    for (const auto& token : instruction->m_tokens) {
        renderToken(token.text, token.type);
    }
}

Instruction::StreamRender::StreamRender(std::ostream& output)
    : m_output(output)
{}

void Instruction::StreamRender::renderToken(const std::string& text, Token::Type token) const {
    m_output << text;
}

const Instruction* DecoderRender::getDecodedInstruction() const {
    return &m_decodedInstruction;
}