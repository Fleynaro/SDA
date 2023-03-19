#include "Test/Core/IRcode/IRcodeFixture.h"
#include "Test/Core/Utils/TestAssertion.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class IRcodeTest : public IRcodeFixture
{
protected:
    pcode::Graph graph;
    ircode::Program program = ircode::Program(&graph);

    ::testing::AssertionResult cmp(ircode::Function* function, const std::string& expectedCode) const {
        std::stringstream ss;
        printIRcode(function, ss, 2);
        return Compare(ss.str(), expectedCode);
    }
};

TEST_F(IRcodeTest, Simplest) {
    // Graph: https://photos.app.goo.gl/qZr7FEV3H7bY9d9Q7
    auto sourcePCode = "\
        rax:8 = COPY 1:8 \n\
        rax:8 = INT_ADD rax:8, 1:8 \
    ";
    auto expectedIRode = "\
        Block B0(level: 1): \n\
            var1[rax]:8 = COPY 0x1:8 \n\
            var2[rax]:8 = INT_ADD var1, 0x1:8 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(function, expectedIRode));
}

TEST_F(IRcodeTest, SimpleConcat) {
    // Graph: https://photos.app.goo.gl/aHmMvgUYDTZnQQTm7
    auto sourcePCode = "\
        rax:8 = COPY 0x2100:8 \n\
        BRANCH <label> \n\
        <label>: \n\
        rax:1 = COPY 0x34:1 \n\
        r10:2 = INT_2COMP rax:2 \n\
    ";
    auto expectedIRode = "\
        Block B0(level: 1, far: B2): \n\
            var1[rax]:8 = COPY 0x2100:8 \n\
        Block B2(level: 2): \n\
            var2[rax]:1 = COPY 0x34:1 \n\
            var3[rax]:2 = EXTRACT var1, 0 \n\
            var4 = CONCAT var3, var2, 0 \n\
            var5[r10]:2 = INT_2COMP var4 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(function, expectedIRode));
}
