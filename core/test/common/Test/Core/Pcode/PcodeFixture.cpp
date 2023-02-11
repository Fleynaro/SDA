#include "PcodeFixture.h"
#include "SDA/Core/Pcode/PcodePrinter.h"
#include "SDA/Core/Pcode/PcodeInstructionProvider.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

std::list<pcode::Instruction> PcodeFixture::parsePcode(const std::string& text) const {
    return pcode::Parser::Parse(text, context->getPlatform()->getRegisterRepository().get());
}

pcode::FunctionGraph* PcodeFixture::parsePcode(const std::string& text, pcode::Graph* graph) const {
    auto instructions = parsePcode(text);
    pcode::ListInstructionProvider provider(instructions);
    graph->explore(0, &provider);
    return graph->getFunctionGraphAt(pcode::InstructionOffset(0));
}

void PcodeFixture::printPcode(pcode::FunctionGraph* funcGraph, std::ostream& out, size_t tabs) const {
    pcode::Printer pcodePrinter(context->getPlatform()->getRegisterRepository().get(), funcGraph->getGraph());
    pcodePrinter.setOutput(out);
    for (size_t i = 0; i < tabs; ++i)
        pcodePrinter.startBlock();
    pcodePrinter.newTabs();
    pcodePrinter.printFunctionGraph(funcGraph);
}