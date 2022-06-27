#include "Decompiler/IRcodeGenerator/IRcodeBlockGenerator.h"

using namespace sda;
using namespace sda::decompiler;

IRcodeBlockGenerator::IRcodeBlockGenerator(ircode::Block* block)
    : m_block(block)
{}

void IRcodeBlockGenerator::executePcode(pcode::Instruction* instr) {

}