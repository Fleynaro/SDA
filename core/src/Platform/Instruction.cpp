#include "Core/Platform/Instruction.h"

using namespace sda;

void Instruction::Printer::print(const Instruction* instruction) const {
    for (const auto& token : instruction->m_tokens) {
        printToken(token.text, token.type);
    }
}

Instruction::StreamPrinter::StreamPrinter(std::ostream& output)
    : m_output(output)
{}

void Instruction::StreamPrinter::printToken(const std::string& text, Token::Type token) const {
    m_output << text;
}