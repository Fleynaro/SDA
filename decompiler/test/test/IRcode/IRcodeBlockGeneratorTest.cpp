#include "Core/IRcode/IRcodeDataTypeProvider.h"
#include "Core/IRcode/IRcodePrinter.h"
#include "Core/Test/Utils/TestAssertion.h"
#include "Decompiler/IRcode/Generator/IRcodeBlockGenerator.h"
#include "Decompiler/Test/PcodeFixture.h"

using namespace sda;
using namespace sda::test;
using namespace sda::decompiler;
using namespace ::testing;

class IRcodeBlockGeneratorTest : public PcodeFixture
{
protected:
    void print(const std::list<std::unique_ptr<ircode::Operation>>& operations, std::ostream& out) const {
        pcode::Printer pcodePrinter(context->getPlatform()->getRegisterRepository());
        ircode::Printer ircodePrinter(&pcodePrinter);
        ircodePrinter.setOutput(out);
        for (const auto& op : operations) {
            ircodePrinter.printOperation(op.get());
            ircodePrinter.newLine();
        }
    }

    ::testing::AssertionResult cmp(const std::list<std::unique_ptr<ircode::Operation>>& operations, const std::string& expectedCode) const {
        std::stringstream ss;
        print(operations, ss);
        return Compare(ss.str(), expectedCode);
    }

    ircode::Block* toIRcode(const std::list<pcode::Instruction>& instructions) const {
        pcode::Block pcodeBlock;
        auto ircodeBlock = new ircode::Block(&pcodeBlock);
        TotalMemorySpace memorySpace;
        ircode::DataTypeProvider dataTypeProvider(context);
        IRcodeBlockGenerator ircodeGen(ircodeBlock, &memorySpace, &dataTypeProvider);
        for (const auto& instr : instructions)
            ircodeGen.executePcode(&instr);
        return ircodeBlock;
    }
};

TEST_F(IRcodeBlockGeneratorTest, Sample1) {
    auto sourcePCode = "\
        rcx:8 = COPY rcx:8 \
        rbx:8 = INT_MULT rdx:8, 4:8 \
        rbx:8 = INT_ADD rcx:8, rbx:8 \
        rbx:8 = INT_ADD rbx:8, 0x10:8 \
        STORE rbx:8, 1.0:8 \
    ";

    auto expectedIRCode = "\
        var1[rcx]:8 = LOAD rcx \
        var2[rcx]:8 = COPY var1 \
        var3[rdx]:8 = LOAD rdx \
        var4[rbx]:8 = INT_MULT var3, 0x4:8 \
        var5[rbx]:8 = INT_ADD var2, var4 \
        var6[rbx]:8 = INT_ADD var5, 0x10:8 \
        var7[var2 + var3 * 4 + 16]:8 = COPY 0x3ff0000000000000:8 \
    ";

    auto instructions = parsePcode(sourcePCode);
    auto& operations = toIRcode(instructions)->getOperations();

    ASSERT_TRUE(cmp(operations, expectedIRCode));
}