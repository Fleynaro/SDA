#include <gtest/gtest.h>
#include "Core/Pcode/PcodeParser.h"
#include "Core/IRcode/IRcodeDataTypeProvider.h"
#include "Core/Test/ContextFixture.h"
#include "Platform/X86/RegisterRepository.h"
#include "Decompiler/IRcode/Generator/IRcodeBlockGenerator.h"

using namespace sda;
using namespace sda::test;
using namespace sda::decompiler;
using namespace ::testing;

class IRcodeBlockGeneratorTest : public ContextFixture
{
    platform::RegisterRepositoryX86 regRepo;
protected:
    std::list<pcode::Instruction> parse(const std::string& text) {
        return pcode::Parser::Parse(text, &regRepo);
    }
};

TEST_F(IRcodeBlockGeneratorTest, Sample1) {
    auto instructions = parse("\
        rcx:8 = COPY rcx:8 \
        rbx:8 = INT_MULT rdx:8, 4:8 \
        rbx:8 = INT_ADD rcx:8, rbx:8 \
        rbx:8 = INT_ADD rbx:8, 0x10:8 \
        STORE rbx:8, 1.0:8 \
    ");

    pcode::Block pcodeBlock;
    ircode::Block ircodeBlock(&pcodeBlock);
    TotalMemorySpace memorySpace;
    ircode::DataTypeProvider dataTypeProvider(context);
    IRcodeBlockGenerator ircodeGen(&ircodeBlock, &memorySpace, &dataTypeProvider);
}