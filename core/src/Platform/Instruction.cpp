#include "SDA/Core/Platform/Instruction.h"

using namespace sda;

void Instruction::Printer::print(const Instruction* instruction) const {
    for (const auto& token : instruction->m_tokens) {
        printToken(token.text, PARENT + token.type);
    }
}

Instruction::StreamPrinter::StreamPrinter(std::ostream& output)
    : m_output(output)
{}

void Instruction::StreamPrinter::printTokenImpl(const std::string& text, Token token) const {
    m_output << text;
}