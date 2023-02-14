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
        Block B4(level: 1): \n\
            NOP \
    ";
    pcode::Graph graph;
    auto instructions = parsePcode(sourcePCode);
    pcode::ListInstructionProvider provider(instructions);
    graph.explore(pcode::InstructionOffset(0, 0), &provider);
    graph.explore(pcode::InstructionOffset(2, 0), &provider);
    auto funcGraph1 = graph.getFunctionGraphAt(pcode::InstructionOffset(0, 0));
    auto funcGraph2 = graph.getFunctionGraphAt(pcode::InstructionOffset(2, 0));
    auto funcGraph3 = graph.getFunctionGraphAt(pcode::InstructionOffset(4, 0));
    ASSERT_TRUE(cmp(funcGraph1, expectedPCodeOfFunc1));
    ASSERT_TRUE(cmp(funcGraph2, expectedPCodeOfFunc2));
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

TEST_F(PcodeTest, Sample6) {
    // Graph: https://photos.app.goo.gl/cbsdJ2VmczicoZ1v5
    auto sourcePCode = "\
        // ---- func1 ---- \n\
        CALL <func2> \n\
        CALL <func3> \n\
        // ---- func2 ---- \n\
        <func2>: \n\
        NOP \n\
        BRANCH <func3> \n\
        // ---- func3 ---- \n\
        <func3>: \n\
        NOP \n\
        BRANCH <func2> \
    ";
    auto expectedPCodeOfFunc1 = "\
        Block B0(level: 1): \n\
            CALL <B2>:8 \n\
            CALL <B4>:8 \
    ";
    auto expectedPCodeOfFunc2 = "\
        Block B2(level: 1, far: B4): \n\
            NOP \n\
            BRANCH <B4>:8 \
    ";
    auto expectedPCodeOfFunc3 = "\
        Block B4(level: 1, far: B2): \n\
            NOP \n\
            BRANCH <B2>:8 \
    ";
    pcode::Graph graph;
    auto instructions = parsePcode(sourcePCode);
    pcode::ListInstructionProvider provider(instructions);
    graph.explore(pcode::InstructionOffset(0, 0), &provider);
    graph.explore(pcode::InstructionOffset(2, 0), &provider);
    auto funcGraph1 = graph.getFunctionGraphAt(pcode::InstructionOffset(0, 0));
    auto funcGraph2 = graph.getFunctionGraphAt(pcode::InstructionOffset(2, 0));
    auto funcGraph3 = graph.getFunctionGraphAt(pcode::InstructionOffset(4, 0));
    auto entryBlock2 = funcGraph2->getEntryBlock();
    auto entryBlock3 = funcGraph3->getEntryBlock();
    ASSERT_EQ(funcGraph1->getReferencesFrom().size(), 2);
    ASSERT_EQ(funcGraph2->getReferencesTo().size(), 1);
    ASSERT_EQ(funcGraph3->getReferencesTo().size(), 1);
    ASSERT_TRUE(cmp(funcGraph1, expectedPCodeOfFunc1));
    ASSERT_TRUE(cmp(funcGraph2, expectedPCodeOfFunc2));
    ASSERT_TRUE(cmp(funcGraph3, expectedPCodeOfFunc3));
    // no loop because blocks belongs to different functions
    ASSERT_FALSE(entryBlock3->hasLoopWith(entryBlock2));
    // remove all links
    entryBlock2->setFarNextBlock(nullptr);
    entryBlock3->setFarNextBlock(nullptr);
    // recover the removed links
    entryBlock3->setFarNextBlock(entryBlock2);
    entryBlock2->setFarNextBlock(entryBlock3);
    // check if there are no changes
    ASSERT_TRUE(cmp(funcGraph1, expectedPCodeOfFunc1));
    ASSERT_TRUE(cmp(funcGraph2, expectedPCodeOfFunc2));
    ASSERT_TRUE(cmp(funcGraph3, expectedPCodeOfFunc3));
    ASSERT_FALSE(entryBlock3->hasLoopWith(entryBlock2));
}

TEST_F(PcodeTest, Sample7) {
    // Graph: https://photos.app.goo.gl/1WcQpH1SoKUUPLU56
    auto sourcePCode = "\
        <func>: \n\
        NOP \n\
        BRANCH <jmp> \n\
        <jmp>: \n\
        CALL <func> \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, far: B2): \n\
            NOP \n\
            BRANCH <B2>:8 \n\
        Block B2(level: 2): \n\
            CALL <B0>:8 \
    ";
    pcode::Graph graph;
    auto funcGraph = parsePcode(sourcePCode, &graph);
    // the function graph refers to itself
    ASSERT_EQ(funcGraph, funcGraph->getReferencesFrom().begin()->second);
    ASSERT_EQ(funcGraph, *funcGraph->getReferencesTo().begin());
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
}

TEST_F(PcodeTest, Sample8) {
    // Graph: https://photos.app.goo.gl/76TRzwXeCEma5TMu8
    auto sourcePCode = "\
        // ---- the main function ---- \n\
        NOP \n\
        CALL <func> \n\
        <newFunc>: \n\
        NOP \n\
        CALL <func> \n\
        RETURN \n\
        // ---- the function that is refered ---- \n\
        <func>: \n\
        RETURN \n\
        // ---- the function that has a new call ---- \n\
        CALL <newFunc> \
    ";
    auto expectedPCodeOfMainFunc = "\
        Block B0(level: 1): \n\
            NOP \n\
            CALL <B5>:8 \n\
            NOP \n\
            CALL <B5>:8 \n\
            RETURN \
    ";
    pcode::Graph graph;
    auto instructions = parsePcode(sourcePCode);
    pcode::ListInstructionProvider provider(instructions);
    graph.explore(pcode::InstructionOffset(0, 0), &provider);
    auto mainFuncGraph = graph.getFunctionGraphAt(pcode::InstructionOffset(0, 0));
    ASSERT_EQ(mainFuncGraph->getReferencesFrom().size(), 2);
    ASSERT_TRUE(cmp(mainFuncGraph, expectedPCodeOfMainFunc));
    // explore the function that has a new call
    graph.explore(pcode::InstructionOffset(6, 0), &provider);
    // now the main function has 1 "from" reference
    ASSERT_EQ(mainFuncGraph->getReferencesFrom().size(), 1);
    // the new function also has 1 "from" reference borrowed from the main function
    auto newFuncGraph = graph.getFunctionGraphAt(pcode::InstructionOffset(2, 0));
    ASSERT_EQ(newFuncGraph->getReferencesFrom().size(), 1);
    // and has 1 "to" reference
    ASSERT_EQ(newFuncGraph->getReferencesTo().size(), 1);
}

// TODO: call (всегда ссылается на entryBlock, но может сослаться на блок с существуюшими refBlocks и тогда надо создать там граф, т.е. сделать блок искуственным entry блоком)
// TODO: two nested loops (with general block) test
// TODO: goto test
// TODO: change graph after explore test
// TODO: joinBlocks test