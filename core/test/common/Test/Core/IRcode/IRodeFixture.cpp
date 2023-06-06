#include "IRcodeFixture.h"
#include "SDA/Core/IRcode/IRcodePrinter.h"
#include "Test/Core/Utils/TestAssertion.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

void IRcodeFixture::SetUp() {
    PcodeFixture::SetUp();
    program = new ircode::Program(graph, globalSymbolTable);
}

void IRcodeFixture::TearDown() {
    delete program;
    PcodeFixture::TearDown();
}

void IRcodeFixture::printIRcode(ircode::Function* function, std::ostream& out, size_t tabs) const {
    pcode::Printer pcodePrinter(context->getPlatform()->getRegisterRepository().get());
    ircode::Printer ircodePrinter(&pcodePrinter);
    ircodePrinter.setOutput(out);
    for (size_t i = 0; i < tabs; ++i)
        ircodePrinter.startBlock();
    ircodePrinter.newTabs();
    ircodePrinter.printFunction(function);
}

ircode::Function* IRcodeFixture::parsePcode(const std::string& text, ircode::Program* program) const {
    auto funcGraph = PcodeFixture::parsePcode(text, program->getGraph());
    return program->toFunction(funcGraph);
}

::testing::AssertionResult IRcodeFixture::cmp(ircode::Function* function, const std::string& expectedCode) const {
    std::stringstream ss;
    printIRcode(function, ss, 2);
    return Compare(ss.str(), expectedCode);
}
