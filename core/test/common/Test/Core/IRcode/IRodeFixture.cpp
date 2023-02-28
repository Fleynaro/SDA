#include "IRcodeFixture.h"
#include "SDA/Core/IRcode/IRcodePrinter.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

void IRcodeFixture::printIRcode(ircode::Function* function, std::ostream& out, size_t tabs) const {
    pcode::Printer pcodePrinter(context->getPlatform()->getRegisterRepository().get());
    ircode::Printer ircodePrinter(&pcodePrinter);
    ircodePrinter.setOutput(out);
    for (size_t i = 0; i < tabs; ++i)
        ircodePrinter.startBlock();
    ircodePrinter.newTabs();
    ircodePrinter.printFunction(function);
}
