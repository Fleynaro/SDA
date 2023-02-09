#include "Test/Core/Pcode/PcodeFixture.h"
#include "Test/Core/Utils/TestAssertion.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class PcodeTest : public PcodeFixture
{
protected:
    ::testing::AssertionResult cmp(pcode::FunctionGraph* funcGraph, const std::string& expectedCode) const {
        std::stringstream ss;
        printPcode(funcGraph, ss, 2);
        return Compare(ss.str(), expectedCode);
    }
};

TEST_F(PcodeTest, Sample1) {
    // Graph: https://photos.app.goo.gl/j2VRr1jGHpA8EaaY9
    auto sourcePCode = "\
        NOP \n\
        <label>: \n\
        NOP \n\
        BRANCH <label> \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B1): \n\
            NOP \n\
        Block B1(level: 2, far: B1): \n\
            NOP \n\
            BRANCH <B1>:8 \
    ";
    pcode::Graph graph;
    parsePcode(sourcePCode, &graph);
    auto funcGraph = graph.getFunctionGraphAt(pcode::InstructionOffset(0));
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
}

TEST_F(PcodeTest, Sample2) {
    // Graph: https://photos.app.goo.gl/SztQ27cwyqCAKGrw9
    auto sourcePCode = "\
        <label>: \n\
        NOP \n\
        CBRANCH <label>, 0:1 \n\
        NOP \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B0): \n\
            NOP \n\
            CBRANCH <B0>:8 \n\
        Block B2(level: 2): \n\
            NOP \
    ";
    pcode::Graph graph;
    parsePcode(sourcePCode, &graph);
    auto funcGraph = graph.getFunctionGraphAt(pcode::InstructionOffset(0));
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
}

TEST_F(PcodeTest, Sample3) {
    // Graph: https://photos.app.goo.gl/REjbhAcUmeLg2rfQ8
    auto sourcePCode = "\
        NOP \n\
        CBRANCH <labelElse>, 0:1 // if condition \n\
        NOP // then block \n\
        BRANCH <labelEnd> \n\
        <labelElse>: \n\
        NOP // else block \n\
        <labelEnd>: \n\
        NOP \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B4): \n\
            NOP \n\
            CBRANCH <B4>:8 \n\
        Block B2(level: 2, far: B5): \n\
            NOP \n\
            BRANCH <B5>:8 \n\
        Block B4(level: 2, near: B5): \n\
            NOP \n\
        Block B5(level: 3): \n\
            NOP \
    ";
    pcode::Graph graph;
    parsePcode(sourcePCode, &graph);
    auto funcGraph = graph.getFunctionGraphAt(pcode::InstructionOffset(0));
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
}
