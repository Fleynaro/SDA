#include "Test/Core/IRcode/IRcodeFixture.h"
#include "Test/Core/Utils/TestAssertion.h"
#include "SDA/Core/Semantics/Semantics.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class SemanticsTest : public IRcodeFixture
{
protected:
    pcode::Graph graph;
    ircode::Program program = ircode::Program(&graph);
};

TEST_F(SemanticsTest, Simplest) {
    auto sourcePCode = "\
        rax:8 = COPY 1:8 \n\
        rax:8 = INT_ADD rax:8, 1:8 \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1): \n\
            var1[rax]:8 = COPY 0x1:8 \n\
            var2[rax]:8 = INT_ADD var1, 0x1:8 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    
}
