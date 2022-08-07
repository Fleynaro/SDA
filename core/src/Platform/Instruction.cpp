#include "Core/Platform/Instruction.h"

using namespace sda;

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