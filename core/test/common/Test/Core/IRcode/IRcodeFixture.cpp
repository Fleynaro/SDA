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

void IRcodeFixture::printIRcode(ircode::Function* function, std::ostream& out, size_t tabs) const {
    pcode::Printer pcodePrinter(context->getPlatform()->getRegisterRepository().get());
    ircode::Printer ircodePrinter(&pcodePrinter);
    ircodePrinter.m_printVarAddressAlways = printVarAddressAlways;
    ircodePrinter.setOutput(out);
    ircodePrinter.setOperationCommentProvider([](const ircode::Operation* operation) -> std::string {
        auto function = operation->getBlock()->getFunction();
        auto output = operation->getOutput();
        if (output == function->getReturnVariable()) {
            return "return";
        } else {
            auto paramVars = function->getParamVariables();
            for (size_t i = 0; i < paramVars.size(); ++i) {
                if (output == paramVars[i]) {
                    auto signatureDt = function->getFunctionSymbol()->getSignature();
                    return signatureDt->getParameters()[i]->getName();
                }
            }
        }
        return "";
    });
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
