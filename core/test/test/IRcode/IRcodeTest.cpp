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

TEST_F(IRcodeTest, Simple) {
    // Graph: https://photos.app.goo.gl/j2VRr1jGHpA8EaaY9
    auto sourcePCode = "\
        rax:8 = INT_ADD rax:8, 1:8 \
    ";
    auto expectedIRode = "\
        Block B0(level: 1, near: B1): \n\
            NOP \n\
        Block B1(level: 2, far: B1): \n\
            NOP \n\
            BRANCH <B1>:8 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(function, expectedIRode));
}

/*
    TODO:
    1) ir-code generator tests
    2) ir-code gen without providers yet, just use block's references to go to parents
*/