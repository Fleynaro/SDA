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

TEST_F(PcodeTest, SimpleLoop) {
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

TEST_F(PcodeTest, SimpleLoopFirstBlock) {
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

TEST_F(PcodeTest, IfElseCondition) {
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

TEST_F(PcodeTest, TwoEntryPointsUnion) {
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

TEST_F(PcodeTest, NewEntryBlockWithLoopFirst) {
    // Graph: https://photos.app.goo.gl/FiG2gJXZK3U69RHXA
    // Description: after the second exploration, the initial entry block is not entry anymore
    auto sourcePCode = "\
        <label1>: \n\
        NOP // block 0 \n\
        BRANCH <label2> \n\
        <label2>: \n\
        NOP // block 2 \n\
        BRANCH <label1> \n\
        BRANCH <label1> // block 4 (new entry block) \
    ";
    auto expectedPCodeBefore = "\
        Block B0(level: 1, far: B2): \n\
            NOP \n\
            BRANCH <B2>:8 \n\
        Block B2(level: 2, far: B0): \n\
            NOP \n\
            BRANCH <B0>:8 \
    ";
    auto expectedPCodeAfter = "\
        Block B4(level: 1, far: B0): \n\
            BRANCH <B0>:8 \n\
        Block B0(level: 2, far: B2): \n\
            NOP \n\
            BRANCH <B2>:8 \n\
        Block B2(level: 3, far: B0): \n\
            NOP \n\
            BRANCH <B0>:8 \
    ";
    pcode::Graph graph;
    auto instructions = parsePcode(sourcePCode);
    pcode::ListInstructionProvider provider(instructions);
    graph.explore(pcode::InstructionOffset(0, 0), &provider);
    auto funcGraphBefore = graph.getFunctionGraphAt(pcode::InstructionOffset(0, 0));
    ASSERT_TRUE(cmp(funcGraphBefore, expectedPCodeBefore));
    graph.explore(pcode::InstructionOffset(4, 0), &provider);
    auto funcGraphAfter = graph.getFunctionGraphAt(pcode::InstructionOffset(4, 0));
    ASSERT_TRUE(cmp(funcGraphAfter, expectedPCodeAfter));
}

TEST_F(PcodeTest, NewEntryBlockWithLoopSecond) {
    // Graph: https://photos.app.goo.gl/QbxjTAtJcmRrPV5i9
    // Description: after the second exploration, the initial entry block is not entry anymore
    // Important: in this test, the intermediate function graph will be created and then removed
    auto sourcePCode = "\
        <label1>: \n\
        NOP // block 0 \n\
        BRANCH <label2> \n\
        <label2>: \n\
        NOP // block 2 \n\
        BRANCH <label1> \n\
        BRANCH <label2> // block 4 (new entry block) \
    ";
    auto expectedPCodeBefore = "\
        Block B0(level: 1, far: B2): \n\
            NOP \n\
            BRANCH <B2>:8 \n\
        Block B2(level: 2, far: B0): \n\
            NOP \n\
            BRANCH <B0>:8 \
    ";
    auto expectedPCodeAfter = "\
        Block B4(level: 1, far: B2): \n\
            BRANCH <B2>:8 \n\
        Block B2(level: 2, far: B0): \n\
            NOP \n\
            BRANCH <B0>:8 \n\
        Block B0(level: 3, far: B2): \n\
            NOP \n\
            BRANCH <B2>:8 \
    ";
    pcode::Graph graph;
    auto instructions = parsePcode(sourcePCode);
    pcode::ListInstructionProvider provider(instructions);
    graph.explore(pcode::InstructionOffset(0, 0), &provider);
    auto funcGraphBefore = graph.getFunctionGraphAt(pcode::InstructionOffset(0, 0));
    ASSERT_TRUE(cmp(funcGraphBefore, expectedPCodeBefore));
    graph.explore(pcode::InstructionOffset(4, 0), &provider);
    auto funcGraphAfter = graph.getFunctionGraphAt(pcode::InstructionOffset(4, 0));
    ASSERT_TRUE(cmp(funcGraphAfter, expectedPCodeAfter));
}

TEST_F(PcodeTest, NestedLoop) {
    // Graph: https://photos.app.goo.gl/T4xNME8267wC5rR57
    auto sourcePCode = "\
        <labelExternalLoop>: \n\
        NOP // block 0 \n\
        <labelInternalLoop>: \n\
        NOP // block 1 \n\
        CBRANCH <labelInternalLoop>, 0:1 \n\
        NOP // block 3 \n\
        CBRANCH <labelExternalLoop>, 0:1 \n\
        NOP // block 5 \n\
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B1): \n\
            NOP \n\
        Block B1(level: 2, near: B3, far: B1): \n\
            NOP \n\
            CBRANCH <B1>:8, 0x0:1 \n\
        Block B3(level: 3, near: B5, far: B0): \n\
            NOP \n\
            CBRANCH <B0>:8, 0x0:1 \n\
        Block B5(level: 4): \n\
            NOP \
    ";
    pcode::Graph graph;
    auto funcGraph = parsePcode(sourcePCode, &graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
}

TEST_F(PcodeTest, NestedLoopWithSharedBlock) {
    // Graph: https://photos.app.goo.gl/PtWzD6NajdQaM2227
    auto sourcePCode = "\
        <labelLoop>: \n\
        NOP // block 1 \n\
        CBRANCH <labelLoop>, 0:1 \n\
        NOP // block 3 \n\
        CBRANCH <labelLoop>, 0:1 \n\
        NOP // block 5 \n\
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B0): \n\
            NOP \n\
            CBRANCH <B0>:8, 0x0:1 \n\
        Block B2(level: 2, near: B4, far: B0): \n\
            NOP \n\
            CBRANCH <B0>:8, 0x0:1 \n\
        Block B4(level: 3): \n\
            NOP \
    ";
    pcode::Graph graph;
    auto funcGraph = parsePcode(sourcePCode, &graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
}

TEST_F(PcodeTest, JoinNextBlock) {
    // Graph: https://photos.app.goo.gl/HQKwWx3KYzA4Y7z2A
    // Description: the next block is joined with the current block.
    auto sourcePCode = "\
        <label>: \n\
        // block 0 (should be joined with the block 1) \n\
        CALL <func> \n\
        // block 1 (should be removed) \n\
        CALL <func> // <--- exploring is started here \n\
        BRANCH <label>  \n\
        // some function \n\
        <func>: \n\
        NOP \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, far: B0): \n\
            CALL <B3>:8 \n\
            CALL <B3>:8 \n\
            BRANCH <B0>:8 \
    ";
    pcode::Graph graph;
    auto instructions = parsePcode(sourcePCode);
    pcode::ListInstructionProvider provider(instructions);
    graph.explore(pcode::InstructionOffset(1, 0), &provider);
    auto funcGraph = graph.getFunctionGraphAt(pcode::InstructionOffset(0, 0));
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    ASSERT_EQ(funcGraph->getReferencesFrom().size(), 2);
}

TEST_F(PcodeTest, IfConditionWithGoto) {
    // Graph: https://photos.app.goo.gl/hdP67sRatS9AJN49A
    auto sourcePCode = "\
        NOP // block 0 \n\
        CBRANCH <labelIf>, 0:1 // if condition \n\
        NOP // then block 2 \n\
        CBRANCH <labelGoto>, 0:1 // goto condition \n\
        <labelIf>: \n\
        NOP // block 4 \n\
        <labelGoto>: \n\
        NOP // block 5 \n\
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B4): \n\
            NOP \n\
            CBRANCH <B4>:8, 0x0:1 \n\
        Block B2(level: 2, near: B4, far: B5): \n\
            NOP \n\
            CBRANCH <B5>:8, 0x0:1 \n\
        Block B4(level: 3, near: B5): \n\
            NOP \n\
        Block B5(level: 4): \n\
            NOP \
    ";
    pcode::Graph graph;
    auto funcGraph = parsePcode(sourcePCode, &graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
}

TEST_F(PcodeTest, IfElseConditionWithNewGoto) {
    // Graph: https://photos.app.goo.gl/UHbX85X6rWQKQMXP9
    // Description: we make goto (new jump) from 'then' block to 'else' block that are on the SAME level (= 2)
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
    // BEFORE:
    auto expectedPCodeBefore = "\
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
    // AFTER:
    auto expectedPCodeAfter = "\
        Block B0(level: 1, near: B2, far: B4): \n\
            NOP \n\
            CBRANCH <B4>:8, 0x0:1 \n\
        Block B4(level: 2, near: B5, far: B2): \n\
            NOP \n\
        Block B2(level: 3, far: B5): \n\
            NOP \n\
            BRANCH <B5>:8 \n\
        Block B5(level: 4): \n\
            NOP \
    ";
    pcode::Graph graph;
    auto funcGraph = parsePcode(sourcePCode, &graph);
    // check that graph is as expected BEFORE modification
    ASSERT_TRUE(cmp(funcGraph, expectedPCodeBefore));
    // modify graph
    auto fromBlock = graph.getBlockAt(pcode::InstructionOffset(4, 0)); // block 4
    auto toBlock = graph.getBlockAt(pcode::InstructionOffset(2, 0)); // block 2
    fromBlock->setFarNextBlock(toBlock);
    // check that graph is as expected AFTER modification
    ASSERT_TRUE(cmp(funcGraph, expectedPCodeAfter));
}

TEST_F(PcodeTest, TwoFunctionsUnitedWithJump) {
    // Graph: https://photos.app.goo.gl/4JuF4vMJxd2DQb2F7
    // Description: we make some changes into the graph and then cancel them, so we should get the same graph as before
    auto sourcePCode = "\
        // ---- func1 ---- \n\
        CALL <func2> \n\
        CALL <func3> \n\
        RETURN \n\
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
            CALL <B3>:8 \n\
            CALL <B5>:8 \n\
            RETURN \
    ";
    auto expectedPCodeOfFunc2 = "\
        Block B3(level: 1, far: B5): \n\
            NOP \n\
            BRANCH <B5>:8 \
    ";
    auto expectedPCodeOfFunc3 = "\
        Block B5(level: 1, far: B3): \n\
            NOP \n\
            BRANCH <B3>:8 \
    ";
    pcode::Graph graph;
    auto instructions = parsePcode(sourcePCode);
    pcode::ListInstructionProvider provider(instructions);
    graph.explore(pcode::InstructionOffset(0, 0), &provider);
    graph.explore(pcode::InstructionOffset(2, 0), &provider);
    auto funcGraph1 = graph.getFunctionGraphAt(pcode::InstructionOffset(0, 0));
    auto funcGraph2 = graph.getFunctionGraphAt(pcode::InstructionOffset(3, 0));
    auto funcGraph3 = graph.getFunctionGraphAt(pcode::InstructionOffset(5, 0));
    auto entryBlock2 = funcGraph2->getEntryBlock();
    auto entryBlock3 = funcGraph3->getEntryBlock();
    ASSERT_EQ(funcGraph1->getReferencesFrom().size(), 2);
    ASSERT_EQ(funcGraph2->getReferencesTo().size(), 1);
    ASSERT_EQ(funcGraph3->getReferencesTo().size(), 1);
    ASSERT_TRUE(cmp(funcGraph1, expectedPCodeOfFunc1));
    ASSERT_TRUE(cmp(funcGraph2, expectedPCodeOfFunc2));
    ASSERT_TRUE(cmp(funcGraph3, expectedPCodeOfFunc3));
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
}

TEST_F(PcodeTest, FunctionRecursion) {
    // Graph: https://photos.app.goo.gl/1WcQpH1SoKUUPLU56
    // Description: we make reference to the function from itself, so we should get the recursion
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

TEST_F(PcodeTest, CallSplitSameBlock) {
    // Graph: https://photos.app.goo.gl/vANPa6oeFFghzje2A
    // Description: the call instruction splits the block in which it is into two blocks making two func. graphs + recursion
    auto sourcePCode = "\
        NOP \n\
        <func>: \n\
        NOP \n\
        CALL <func> \
        NOP \
    ";
    auto expectedPCodeOfFunc1 = "\
        Block B0(level: 1, near: B1): \n\
            NOP \
    ";
    auto expectedPCodeOfFunc2 = "\
        Block B1(level: 1): \n\
            NOP \n\
            CALL <B1>:8 \n\
            NOP \
    ";
    pcode::Graph graph;
    auto instructions = parsePcode(sourcePCode);
    pcode::ListInstructionProvider provider(instructions);
    graph.explore(pcode::InstructionOffset(0, 0), &provider);
    auto funcGraph1 = graph.getFunctionGraphAt(pcode::InstructionOffset(0, 0));
    auto funcGraph2 = graph.getFunctionGraphAt(pcode::InstructionOffset(1, 0));
    // the second function graph refers to itself (recursion)
    ASSERT_EQ(funcGraph2, *funcGraph2->getReferencesTo().begin());
    ASSERT_TRUE(cmp(funcGraph1, expectedPCodeOfFunc1));
    ASSERT_TRUE(cmp(funcGraph2, expectedPCodeOfFunc2));
}

TEST_F(PcodeTest, NewCallSplitBlock) {
    // Graph: https://photos.app.goo.gl/76TRzwXeCEma5TMu8
    // Description: we add a new call to the middle of the block splitting it, so we should get the new block
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

TEST_F(PcodeTest, ProgramWithIfElseLoop) {
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
        NOP // block b \
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

TEST_F(PcodeTest, ProgramWithBreakContinueInLoop) {
    // Graph: https://photos.app.goo.gl/eV5TcP5nEH95ZJ1a8
    auto sourcePCode = "\
        <labelStart>: \n\
        NOP // block 0 \n\
        CBRANCH <labelIf>, 0:1 // if condition \n\
        NOP // then block 2 \n\
        CBRANCH <labelEnd>, 0:1 // break \n\
        <labelIf>: \n\
        CBRANCH <labelStart>, 0:1 // continue \n\
        NOP // block 5 \n\
        CBRANCH <labelStart>, 0:1 // while jump \n\
        <labelEnd>: \n\
        NOP // block 7 \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B4): \n\
            NOP \n\
            CBRANCH <B4>:8, 0x0:1 \n\
        Block B2(level: 2, near: B4, far: B7): \n\
            NOP \n\
            CBRANCH <B7>:8, 0x0:1 \n\
        Block B4(level: 3, near: B5, far: B0): \n\
            CBRANCH <B0>:8, 0x0:1 \n\
        Block B5(level: 4, near: B7, far: B0): \n\
            NOP \n\
            CBRANCH <B0>:8, 0x0:1 \n\
        Block B7(level: 5): \n\
            NOP \
    ";
    pcode::Graph graph;
    auto funcGraph = parsePcode(sourcePCode, &graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
}

// TODO: during test do update of blocks two times like in React

// Потом (ir-code генерация)
// - мы должны при смене связей p-code блоков мы должны автоматически запускать процедуру генерации ir-code этих блоков
// - должен быть прямой маппинг между p-code и ir-code блоками
// - при структурном анализе мы можем делать дублирование маленьких блоков для избежания goto, но не делать этого в ircode! ir-code не для визуализации, а для анализа