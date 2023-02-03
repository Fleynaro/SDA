#include "PcodeFixture.h"
#include "SDA/Core/Pcode/PcodePrinter.h"
#include "SDA/Decompiler/Pcode/PcodeGraphBuilder.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

std::list<pcode::Instruction> PcodeFixture::parsePcode(const std::string& text) const {
    return pcode::Parser::Parse(text, context->getPlatform()->getRegisterRepository().get());
}

void PcodeFixture::parsePcode(const std::string& text, pcode::Graph* graph) const {
    auto instructions = parsePcode(text);
    auto blockBuilder = std::make_shared<decompiler::PcodeBlockBuilder>(graph, instructions);
    decompiler::PcodeGraphBuilder builder(graph, blockBuilder);
    builder.start({ pcode::InstructionOffset(0) }, true);
}

void PcodeFixture::printPcode(pcode::FunctionGraph* graph, std::ostream& out) const {
    pcode::Printer pcodePrinter(context->getPlatform()->getRegisterRepository().get());
    pcodePrinter.setOutput(out);
    pcodePrinter.printFunctionGraph(graph);
}