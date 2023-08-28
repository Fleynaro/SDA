#include "SDA/Core/Pcode/PcodePrinter.h"
#include "Test/Core/Pcode/PcodeFixture.h"
#include "Test/Core/Utils/TestAssertion.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class PcodeStructurerTest : public PcodeFixture
{
protected:
    pcode::StructTree toStructTree(pcode::FunctionGraph* funcGraph) {
        pcode::StructTree structTree;
        pcode::Structurer structurer(funcGraph, &structTree);
        structurer.start();
        return structTree;
    }

    ::testing::AssertionResult cmpStructTree(pcode::StructTree* structTree, const std::string& expectedCode) const {
        std::stringstream ss;
        pcode::StructTreePrinter printer;
        printer.setOutput(ss);
        pcode::Printer pcodePrinter(context->getPlatform()->getRegisterRepository().get());
        pcodePrinter.setParentPrinter(&printer);
        printer.setCodePrinter(pcodePrinter.getCodePrinter());
        printer.setConditionPrinter(pcodePrinter.getConditionPrinter());
        printer.printStructTree(structTree);
        return Compare(ss.str(), expectedCode);
    }
};

TEST_F(PcodeStructurerTest, Sequence) {
    // Graph: https://photos.app.goo.gl/Jooi1THBgyMpMtdV6
    auto sourcePCode = "\
        NOP \n\
        BRANCH <label> \n\
        <label>: \n\
        RETURN \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, far: B2): \n\
            NOP \n\
            BRANCH <B2>:8 \n\
        Block B2(level: 2): \n\
            RETURN \
    ";
    auto expectedStructCode = "\
        // Block B0 \n\
        NOP \n\
        // Block B2 \n\
        RETURN \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}

TEST_F(PcodeStructurerTest, If) {
    // Graph: https://photos.app.goo.gl/MbKE2Nh2YoxFKZV57
    auto sourcePCode = "\
        NOP // cond block \n\
        CBRANCH <label>, 0x0:1 \n\
        NOP // then block \n\
        <label>: \n\
        RETURN \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B3): \n\
            NOP \n\
            CBRANCH <B3>:8, 0x0:1 \n\
        Block B2(level: 2, near: B3): \n\
            NOP \n\
        Block B3(level: 3): \n\
            RETURN \
    ";
    auto expectedStructCode = "\
        // Block B0 \n\
        NOP \n\
        if (!0x0:1) { \n\
            // Block B2 \n\
            NOP \n\
        } \n\
        // Block B3 \n\
        RETURN \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}

TEST_F(PcodeStructurerTest, IfElse) {
    // Graph: https://photos.app.goo.gl/JDqbyyouYmj867Fj9
    auto sourcePCode = "\
        NOP // cond block \n\
        CBRANCH <label1>, 0x0:1 \n\
        NOP // then block \n\
        BRANCH <label2> \n\
        <label1>: \n\
        NOP // else block \n\
        <label2>: \n\
        RETURN \
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
            RETURN \
    ";
    auto expectedStructCode = "\
        // Block B0 \n\
        NOP \n\
        if (!0x0:1) { \n\
            // Block B2 \n\
            NOP \n\
        } else { \n\
            // Block B4 \n\
            NOP \n\
        } \n\
        // Block B5 \n\
        RETURN \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}

TEST_F(PcodeStructurerTest, IfNested) {
    // Graph: https://photos.app.goo.gl/be3Vxd1mPqqQWXF69
    auto sourcePCode = "\
        NOP // cond block 1 \n\
        CBRANCH <end>, 0x0:1 \n\
        NOP // cond block 2 \n\
        CBRANCH <end>, 0x1:1 \n\
        NOP // then block \n\
        <end>: \n\
        RETURN \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B5): \n\
            NOP \n\
            CBRANCH <B5>:8, 0x0:1 \n\
        Block B2(level: 2, near: B4, far: B5): \n\
            NOP \n\
            CBRANCH <B5>:8, 0x1:1 \n\
        Block B4(level: 3, near: B5): \n\
            NOP \n\
        Block B5(level: 4): \n\
            RETURN \
    ";
    auto expectedStructCode = "\
        // Block B0 \n\
        NOP \n\
        if (!0x0:1) { \n\
            // Block B2 \n\
            NOP \n\
            if (!0x1:1) { \n\
                // Block B4 \n\
                NOP \n\
            } \n\
        } \n\
        // Block B5 \n\
        RETURN \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}

TEST_F(PcodeStructurerTest, IfOr) {
    // Graph: https://photos.app.goo.gl/B15S8ruzyXBbPrHh8
    auto sourcePCode = "\
        NOP // cond block 1 \n\
        CBRANCH <then>, 0x0:1 \n\
        NOP // cond block 2 \n\
        CBRANCH <then>, 0x1:1 \n\
        NOP // cond block 3 \n\
        CBRANCH <end>, 0x2:1 \n\
        <then>: \n\
        NOP // then block \n\
        <end>: \n\
        RETURN \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B6): \n\
            NOP \n\
            CBRANCH <B6>:8, 0x0:1 \n\
        Block B2(level: 2, near: B4, far: B6): \n\
            NOP \n\
            CBRANCH <B6>:8, 0x1:1 \n\
        Block B4(level: 3, near: B6, far: B7): \n\
            NOP \n\
            CBRANCH <B7>:8, 0x2:1 \n\
        Block B6(level: 4, near: B7): \n\
            NOP \n\
        Block B7(level: 5): \n\
            RETURN \
    ";
    auto expectedStructCode = "\
        // Block B0 \n\
        NOP \n\
        if (!0x0:1) { \n\
            // Block B2 \n\
            NOP \n\
            if (!0x1:1) { \n\
                // Block B4 \n\
                NOP \n\
                if (0x2:1) { \n\
                    goto label_B7; \n\
                } \n\
            } \n\
        } \n\
        // Block B6 \n\
        NOP \n\
        label_B7: \n\
        // Block B7 \n\
        RETURN \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}

TEST_F(PcodeStructurerTest, WhileNoCond) {
    // Graph: https://photos.app.goo.gl/stAtXXNNkK1trByA6
    auto sourcePCode = "\
        <label>: \n\
        NOP \n\
        BRANCH <label> \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, far: B0): \n\
            NOP \n\
            BRANCH <B0>:8 \
    ";
    auto expectedStructCode = "\
        while (true) { \n\
            // Block B0 \n\
            NOP \n\
        } \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}

TEST_F(PcodeStructurerTest, WhilePreCond) {
    // Graph: https://photos.app.goo.gl/reHimnmY42D8QK8U8
    auto sourcePCode = "\
        <loop>: \n\
        CBRANCH <end>, 0x0:1 \
        NOP \n\
        BRANCH <loop> \n\
        <end>: \n\
        RETURN \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B1, far: B3): \n\
            CBRANCH <B3>:8, 0x0:1 \n\
        Block B1(level: 2, far: B0): \n\
            NOP \n\
            BRANCH <B0>:8 \n\
        Block B3(level: 2): \n\
            RETURN \
    ";
    auto expectedStructCode = "\
        while (true) { \n\
            // Block B0 \n\
            if (0x0:1) { \n\
                break; \n\
            } \n\
            // Block B1 \n\
            NOP \n\
        } \n\
        // Block B3 \n\
        RETURN \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}

TEST_F(PcodeStructurerTest, WhilePostCond) {
    // Graph: https://photos.app.goo.gl/qoXXTN5owzcC6zqV7
    auto sourcePCode = "\
        <loop>: \n\
        NOP \n\
        CBRANCH <loop>, 0x0:1 \n\
        RETURN \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B0): \n\
            NOP \n\
            CBRANCH <B0>:8, 0x0:1 \n\
        Block B2(level: 2): \n\
            RETURN \
    ";
    auto expectedStructCode = "\
        while (true) { \n\
            // Block B0 \n\
            NOP \n\
            if (!0x0:1) { \n\
                break; \n\
            } \n\
        } \n\
        // Block B2 \n\
        RETURN \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}

TEST_F(PcodeStructurerTest, WhilePrePostCond) {
    // Graph: https://photos.app.goo.gl/oQmpZDSd1cEf2vi29
    auto sourcePCode = "\
        <loop>: \n\
        NOP \n\
        CBRANCH <end>, 0x0:1 \
        NOP \n\
        CBRANCH <loop>, 0x1:1 \n\
        <end>: \n\
        RETURN \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B4): \n\
            NOP \n\
            CBRANCH <B4>:8, 0x0:1 \n\
        Block B2(level: 2, near: B4, far: B0): \n\
            NOP \n\
            CBRANCH <B0>:8, 0x1:1 \n\
        Block B4(level: 3): \n\
            RETURN \
    ";
    auto expectedStructCode = "\
        while (true) { \n\
            // Block B0 \n\
            NOP \n\
            if (0x0:1) { \n\
                break; \n\
            } \n\
            // Block B2 \n\
            NOP \n\
            if (!0x1:1) { \n\
                break; \n\
            } \n\
        } \n\
        // Block B4 \n\
        RETURN \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}

TEST_F(PcodeStructurerTest, WhileContinue) {
    // Graph: https://photos.app.goo.gl/JmGmjfyuHsctAjK99
    auto sourcePCode = "\
        <loop>: \n\
        NOP \n\
        CBRANCH <end>, 0x0:1 \
        NOP \n\
        CBRANCH <loop>, 0x1:1 \n\
        NOP \n\
        BRANCH <loop> \n\
        <end>: \n\
        RETURN \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B6): \n\
            NOP \n\
            CBRANCH <B6>:8, 0x0:1 \n\
        Block B2(level: 2, near: B4, far: B0): \n\
            NOP \n\
            CBRANCH <B0>:8, 0x1:1 \n\
        Block B6(level: 2): \n\
            RETURN \n\
        Block B4(level: 3, far: B0): \n\
            NOP \n\
            BRANCH <B0>:8 \
    ";
    auto expectedStructCode = "\
        while (true) { \n\
            // Block B0 \n\
            NOP \n\
            if (0x0:1) { \n\
                break; \n\
            } \n\
            // Block B2 \n\
            NOP \n\
            if (0x1:1) { \n\
                continue; \n\
            } \n\
            // Block B4 \n\
            NOP \n\
        } \n\
        // Block B6 \n\
        RETURN \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}

TEST_F(PcodeStructurerTest, WhileIf) {
    // Graph: https://photos.app.goo.gl/23YhVeEhmn9e3RNEA
    auto sourcePCode = "\
        <loop>: \n\
        NOP \n\
        CBRANCH <if>, 0x0:1 \
        NOP \n\
        <if>: \n\
        NOP \n\
        CBRANCH <loop>, 0x1:1 \n\
        RETURN \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B3): \n\
            NOP \n\
            CBRANCH <B3>:8, 0x0:1 \n\
        Block B2(level: 2, near: B3): \n\
            NOP \n\
        Block B3(level: 3, near: B5, far: B0): \n\
            NOP \n\
            CBRANCH <B0>:8, 0x1:1 \n\
        Block B5(level: 4): \n\
            RETURN \
    ";
    auto expectedStructCode = "\
        while (true) { \n\
            // Block B0 \n\
            NOP \n\
            if (!0x0:1) { \n\
                // Block B2 \n\
                NOP \n\
            } \n\
            // Block B3 \n\
            NOP \n\
            if (!0x1:1) { \n\
                break; \n\
            } \n\
        } \n\
        // Block B5 \n\
        RETURN \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}

TEST_F(PcodeStructurerTest, WhileIfElseBreakContinue) {
    // Graph: https://photos.app.goo.gl/CjknXHqJrx8w2V8a8
    auto sourcePCode = "\
        <loop>: \n\
        NOP \n\
        CBRANCH <end>, 0x0:1 \n\
        NOP \n\
        CBRANCH <if_else>, 0x1:1 \n\
        NOP // then (break) \n\
        CBRANCH <end>, 0x2:1 \n\
        BRANCH <if_end> \n\
        <if_else>: \n\
        NOP // else (continue) \n\
        CBRANCH <loop>, 0x3:1 \n\
        <if_end>: \n\
        NOP \n\
        CBRANCH <loop>, 0x4:1 \n\
        <end>: \n\
        RETURN \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: Bb): \n\
            NOP \n\
            CBRANCH <Bb>:8, 0x0:1 \n\
        Block B2(level: 2, near: B4, far: B7): \n\
            NOP \n\
            CBRANCH <B7>:8, 0x1:1 \n\
        Block B4(level: 3, near: B6, far: Bb): \n\
            NOP \n\
            CBRANCH <Bb>:8, 0x2:1 \n\
        Block B7(level: 3, near: B9, far: B0): \n\
            NOP \n\
            CBRANCH <B0>:8, 0x3:1 \n\
        Block B6(level: 4, far: B9): \n\
            BRANCH <B9>:8 \n\
        Block B9(level: 5, near: Bb, far: B0): \n\
            NOP \n\
            CBRANCH <B0>:8, 0x4:1 \n\
        Block Bb(level: 6): \n\
            RETURN \
    ";
    auto expectedStructCode = "\
        while (true) { \n\
            // Block B0 \n\
            NOP \n\
            if (0x0:1) { \n\
                break; \n\
            } \n\
            // Block B2 \n\
            NOP \n\
            if (!0x1:1) { \n\
                // Block B4 \n\
                NOP \n\
                if (0x2:1) { \n\
                    break; \n\
                } \n\
                // Block B6 \n\
            } else { \n\
                // Block B7 \n\
                NOP \n\
                if (0x3:1) { \n\
                    continue; \n\
                } \n\
            } \n\
            // Block B9 \n\
            NOP \n\
            if (!0x4:1) { \n\
                break; \n\
            } \n\
        } \n\
        // Block Bb \n\
        RETURN \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}

TEST_F(PcodeStructurerTest, WhileNested) {
    // Graph: https://photos.app.goo.gl/tQ1neYL8HtRnnUDS7
    auto sourcePCode = "\
        <loop1>: \n\
        NOP \n\
        CBRANCH <end_loop1>, 0x0:1 \
        <loop2>: \n\
        NOP \n\
        CBRANCH <end_loop2>, 0x1:1 \n\
        NOP \n\
        CBRANCH <loop2>, 0x2:1 \n\
        <end_loop2>: \n\
        NOP \n\
        CBRANCH <loop1>, 0x3:1 \n\
        <end_loop1>: \n\
        RETURN \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B8): \n\
            NOP \n\
            CBRANCH <B8>:8, 0x0:1 \n\
        Block B2(level: 2, near: B4, far: B6): \n\
            NOP \n\
            CBRANCH <B6>:8, 0x1:1 \n\
        Block B4(level: 3, near: B6, far: B2): \n\
            NOP \n\
            CBRANCH <B2>:8, 0x2:1 \n\
        Block B6(level: 4, near: B8, far: B0): \n\
            NOP \n\
            CBRANCH <B0>:8, 0x3:1 \n\
        Block B8(level: 5): \n\
            RETURN \
    ";
    auto expectedStructCode = "\
        while (true) { \n\
            // Block B0 \n\
            NOP \n\
            if (0x0:1) { \n\
                break; \n\
            } \n\
            while (true) { \n\
                // Block B2 \n\
                NOP \n\
                if (0x1:1) { \n\
                    break; \n\
                } \n\
                // Block B4 \n\
                NOP \n\
                if (!0x2:1) { \n\
                    break; \n\
                } \n\
            } \n\
            // Block B6 \n\
            NOP \n\
            if (!0x3:1) { \n\
                break; \n\
            } \n\
        } \n\
        // Block B8 \n\
        RETURN \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}

TEST_F(PcodeStructurerTest, WhileReturn) {
    // Graph: https://photos.app.goo.gl/Co1RNGgyhbYcwh1b7
    auto sourcePCode = "\
        <loop>: \n\
        NOP \n\
        CBRANCH <end>, 0x0:1 \
        NOP \n\
        CBRANCH <loop>, 0x1:1 \n\
        NOP \n\
        <end>: \n\
        RETURN \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B5): \n\
            NOP \n\
            CBRANCH <B5>:8, 0x0:1 \n\
        Block B2(level: 2, near: B4, far: B0): \n\
            NOP \n\
            CBRANCH <B0>:8, 0x1:1 \n\
        Block B4(level: 3, near: B5): \n\
            NOP \n\
        Block B5(level: 4): \n\
            RETURN \
    ";
    auto expectedStructCode = "\
        while (true) { \n\
            // Block B0 \n\
            NOP \n\
            if (0x0:1) { \n\
                goto label_B5; \n\
            } \n\
            // Block B2 \n\
            NOP \n\
            if (!0x1:1) { \n\
                break; \n\
            } \n\
        } \n\
        // Block B4 \n\
        NOP \n\
        label_B5: \n\
        // Block B5 \n\
        RETURN \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}

TEST_F(PcodeStructurerTest, GotoToWhile) {
    // Graph: https://photos.app.goo.gl/A6Nro98kTX33gB5b7
    auto sourcePCode = "\
        NOP \n\
        CBRANCH <label>, 0x0:1 \
        <loop>: \n\
        NOP \n\
        <label>: \n\
        NOP \n\
        CBRANCH <loop>, 0x1:1 \n\
        RETURN \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B3): \n\
            NOP \n\
            CBRANCH <B3>:8, 0x0:1 \n\
        Block B2(level: 2, near: B3): \n\
            NOP \n\
        Block B3(level: 3, near: B5, far: B2): \n\
            NOP \n\
            CBRANCH <B2>:8, 0x1:1 \n\
        Block B5(level: 4): \n\
            RETURN \
    ";
    auto expectedStructCode = "\
        // Block B0 \n\
        NOP \n\
        if (0x0:1) { \n\
            goto label_B3; \n\
        } \n\
        while (true) { \n\
            // Block B2 \n\
            NOP \n\
            label_B3: \n\
            // Block B3 \n\
            NOP \n\
            if (!0x1:1) { \n\
                break; \n\
            } \n\
        } \n\
        // Block B5 \n\
        RETURN \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}

TEST_F(PcodeStructurerTest, IfReturn) {
    // Graph: https://photos.app.goo.gl/J4AiZ7v8mNEVujGYA
    auto sourcePCode = "\
        NOP \n\
        CBRANCH <end>, 0x0:1 \
        NOP \n\
        CBRANCH <if_end>, 0x1:1 \
        NOP \n\
        CBRANCH <end>, 0x2:1 \
        <if_end>: \n\
        NOP \n\
        <end>: \n\
        RETURN \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B7): \n\
            NOP \n\
            CBRANCH <B7>:8, 0x0:1 \n\
        Block B2(level: 2, near: B4, far: B6): \n\
            NOP \n\
            CBRANCH <B6>:8, 0x1:1 \n\
        Block B4(level: 3, near: B6, far: B7): \n\
            NOP \n\
            CBRANCH <B7>:8, 0x2:1 \n\
        Block B6(level: 4, near: B7): \n\
            NOP \n\
        Block B7(level: 5): \n\
            RETURN \
    ";
    auto expectedStructCode = "\
        // Block B0 \n\
        NOP \n\
        if (!0x0:1) { \n\
            // Block B2 \n\
            NOP \n\
            if (!0x1:1) { \n\
                // Block B4 \n\
                NOP \n\
                if (0x2:1) { \n\
                    goto label_B7; \n\
                } \n\
            } \n\
            // Block B6 \n\
            NOP \n\
        } \n\
        label_B7: \n\
        // Block B7 \n\
        RETURN \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}

TEST_F(PcodeStructurerTest, ComplexGoto) {
    // Graph: https://photos.app.goo.gl/TPqg2YBgukZNK57c6
    auto sourcePCode = "\
        NOP \n\
        CBRANCH <label1>, 0x0:1 \
        NOP \n\
        CBRANCH <label2>, 0x1:1 \
        <label1>: \n\
        NOP \n\
        BRANCH <label3> \n\
        <label2>: \n\
        CBRANCH <end>, 0x2:1 \
        <label3>: \n\
        NOP \n\
        <end>: \n\
        RETURN \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B4): \n\
            NOP \n\
            CBRANCH <B4>:8, 0x0:1 \n\
        Block B2(level: 2, near: B4, far: B6): \n\
            NOP \n\
            CBRANCH <B6>:8, 0x1:1 \n\
        Block B4(level: 3, far: B7): \n\
            NOP \n\
            BRANCH <B7>:8 \n\
        Block B6(level: 3, near: B7, far: B8): \n\
            CBRANCH <B8>:8, 0x2:1 \n\
        Block B7(level: 4, near: B8): \n\
            NOP \n\
        Block B8(level: 5): \n\
            RETURN \
    ";
    auto expectedStructCode = "\
        // Block B0 \n\
        NOP \n\
        if (!0x0:1) { \n\
            // Block B2 \n\
            NOP \n\
            if (0x1:1) { \n\
                goto label_B6; \n\
            } \n\
        } \n\
        // Block B4 \n\
        NOP \n\
        goto label_B7; \n\
        label_B6: \n\
        // Block B6 \n\
        if (!0x2:1) { \n\
            label_B7: \n\
            // Block B7 \n\
            NOP \n\
        } \n\
        // Block B8 \n\
        RETURN \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}

TEST_F(PcodeStructurerTest, ComplexGoto2) {
    // Graph: https://photos.app.goo.gl/JFPNjAj9DmTFzo3p6
    // TODO: this graph can be collapsed into a tree with 1 goto instead of 2
    auto sourcePCode = "\
        NOP \n\
        CBRANCH <label1>, 0x0:1 \n\
        NOP \n\
        CBRANCH <label2>, 0x1:1 \n\
        <label1>: \n\
        NOP \n\
        BRANCH <end> \n\
        <label2>: \n\
        NOP \n\
        <end>: \n\
        NOP \n\
        RETURN \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B2, far: B4): \n\
            NOP \n\
            CBRANCH <B4>:8, 0x0:1 \n\
        Block B2(level: 2, near: B4, far: B6): \n\
            NOP \n\
            CBRANCH <B6>:8, 0x1:1 \n\
        Block B4(level: 3, far: B7): \n\
            NOP \n\
            BRANCH <B7>:8 \n\
        Block B6(level: 3, near: B7): \n\
            NOP \n\
        Block B7(level: 4): \n\
            NOP \n\
            RETURN \
    ";
    auto expectedStructCode = "\
        // Block B0 \n\
        NOP \n\
        if (!0x0:1) { \n\
            // Block B2 \n\
            NOP \n\
            if (0x1:1) { \n\
                goto label_B6; \n\
            } \n\
        } \n\
        // Block B4 \n\
        NOP \n\
        label_B7: \n\
        // Block B7 \n\
        NOP \n\
        RETURN \n\
        label_B6: \n\
        // Block B6 \n\
        NOP \n\
        goto label_B7; \
    ";
    
    auto funcGraph = parsePcode(sourcePCode, graph);
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
    auto structTree = toStructTree(funcGraph);
    ASSERT_TRUE(cmpStructTree(&structTree, expectedStructCode));
}
