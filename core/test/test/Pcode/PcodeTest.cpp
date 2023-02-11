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
        NOP // block 0 \n\
        <label>: \n\
        NOP // block 1 \n\
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
        NOP // block 0 \n\
        CBRANCH <label>, 0:1 \n\
        NOP // block 2 \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B0): \n\
            NOP \n\
            CBRANCH <B0>:8, 0x0:1 \n\
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
        NOP // block 0 \n\
        CBRANCH <labelElse>, 0:1 // if-else condition \n\
        NOP // then block 2 \n\
        BRANCH <labelEnd> \n\
        <labelElse>: \n\
        NOP // else block 4 \n\
        <labelEnd>: \n\
        NOP // block 5 \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B4): \n\
            NOP \n\
            CBRANCH <B4>:8, 0x0:1 \n\
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

TEST_F(PcodeTest, Sample4) {
    // Graph: 
    auto sourcePCode = "\
        NOP // block 0 \n\
        CBRANCH <labelIf>, 0:1 // if condition \n\
        NOP // then block \n\
        <labelIf>: \n\
        NOP \n\
        <labelWhile>: \n\
        NOP \n\
        CBRANCH <labelElse>, 0:1 // if-else condition \n\
        NOP // then block \n\
        BRANCH <labelEnd> \n\
        <labelElse>: \n\
        NOP // else block \n\
        <labelEnd>: \n\
        NOP \n\
        CBRANCH <labelWhile>, 0:1 \n\
        NOP \n\
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B4): \n\
    ";
    pcode::Graph graph;
    parsePcode(sourcePCode, &graph);
    auto funcGraph = graph.getFunctionGraphAt(pcode::InstructionOffset(0));
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
}