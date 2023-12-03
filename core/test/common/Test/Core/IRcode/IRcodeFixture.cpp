#include "IRcodeFixture.h"
#include "SDA/Core/IRcode/IRcodePrinter.h"
#include "SDA/Platform/X86/CallingConvention.h"
#include "Test/Core/Utils/TestAssertion.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

void IRcodeFixture::SetUp() {
    PcodeFixture::SetUp();
    program = new ircode::Program(graph, globalSymbolTable);
    auto callConv = std::make_shared<platform::FastcallCallingConvention>();
    ctxSync = std::make_unique<ircode::ContextSync>(program, globalSymbolTable, callConv);
    pcodeSync = std::make_unique<ircode::PcodeSync>(program);
    eventPipe->connect(ctxSync->getEventPipe());
    eventPipe->connect(pcodeSync->getEventPipe());
}

void IRcodeFixture::TearDown() {
    delete program;
    PcodeFixture::TearDown();
}

ircode::Function* IRcodeFixture::parsePcode(const std::string& text, ircode::Program* program) const {
    auto funcGraph = PcodeFixture::parsePcode(text, program->getGraph());
    return program->toFunction(funcGraph);
}

::testing::AssertionResult IRcodeFixture::cmp(ircode::Function* function, const std::string& expectedCode) const {
    return Compare(PrintIRcode(function, 2, printVarAddressAlways), expectedCode);
}
