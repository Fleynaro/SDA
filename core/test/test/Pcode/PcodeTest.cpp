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
    auto funcGraph = parsePcode(sourcePCode, &graph);
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
    auto funcGraph = parsePcode(sourcePCode, &graph);
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
    auto funcGraph = parsePcode(sourcePCode, &graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
}

TEST_F(PcodeTest, Sample4) {
    // Graph: https://photos.app.goo.gl/rHtPCcVZxJdTStND7
    auto sourcePCode = "\
        NOP // block 0 (func 1) \n\
        BRANCH <label> \n\
        NOP // block 2 (func 2) \n\
        BRANCH <label> \n\
        <label>: \n\
        NOP // block 4 (func 3) \
    ";
    auto expectedPCodeOfFunc1 = "\
        Block B0(level: 1, far: B4): \n\
            NOP \n\
            BRANCH <B4>:8 \
    ";
    auto expectedPCodeOfFunc2 = "\
        Block B2(level: 1, far: B4): \n\
            NOP \n\
            BRANCH <B4>:8 \
    ";
    auto expectedPCodeOfFunc3 = "\
        Block B4(level: 2): \n\
            NOP \
    ";
    pcode::Graph graph;
    auto instructions = parsePcode(sourcePCode);
    pcode::ListInstructionProvider provider(instructions);
    graph.explore(pcode::InstructionOffset(0, 0), &provider);
    graph.explore(pcode::InstructionOffset(2, 0), &provider);
    auto funcGraph1 = graph.getFunctionGraphAt(pcode::InstructionOffset(0, 0));
    ASSERT_TRUE(cmp(funcGraph1, expectedPCodeOfFunc1));
    auto funcGraph2 = graph.getFunctionGraphAt(pcode::InstructionOffset(2, 0));
    ASSERT_TRUE(cmp(funcGraph2, expectedPCodeOfFunc2));
    auto funcGraph3 = graph.getFunctionGraphAt(pcode::InstructionOffset(4, 0));
    ASSERT_TRUE(cmp(funcGraph3, expectedPCodeOfFunc3));
}

TEST_F(PcodeTest, Sample5) {
    // Graph: https://photos.app.goo.gl/tXkvQ2kKjG2ZGoC5A
    auto sourcePCode = "\
        NOP // block 0 \n\
        CBRANCH <labelIf>, 0:1 // if condition \n\
        NOP // then block 2 \n\
        <labelIf>: \n\
        NOP \n\
        <labelWhile>: \n\
        NOP \n\
        CBRANCH <labelElse>, 0:1 // if-else condition \n\
        NOP // then block 6 \n\
        BRANCH <labelEnd> \n\
        <labelElse>: \n\
        NOP // else block 8 \n\
        <labelEnd>: \n\
        NOP \n\
        CBRANCH <labelWhile>, 0:1 \n\
        NOP // block b \n\
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B3): \n\
            NOP \n\
            CBRANCH <B3>:8, 0x0:1 \n\
        Block B2(level: 2, near: B3): \n\
            NOP \n\
        Block B3(level: 3, near: B4): \n\
            NOP \n\
        Block B4(level: 4, near: B6, far: B8): \n\
            NOP \n\
            CBRANCH <B8>:8, 0x0:1 \n\
        Block B6(level: 5, far: B9): \n\
            NOP \n\
            BRANCH <B9>:8 \n\
        Block B8(level: 5, near: B9): \n\
            NOP \n\
        Block B9(level: 6, near: Bb, far: B4): \n\
            NOP \n\
            CBRANCH <B4>:8, 0x0:1 \n\
        Block Bb(level: 7): \n\
            NOP \
    ";
    pcode::Graph graph;
    auto funcGraph = parsePcode(sourcePCode, &graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
}