#include "Test/Core/Semantics/SemanticsFixture.h"
#include "SDA/Core/Semantics/DataFlowSemantics.h"
#include "SDA/Core/Utils/IOManip.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class DataFlowSemanticsTest : public SemanticsFixture
{
protected:
    std::unique_ptr<semantics::DataFlowRepository> dataFlowRepo;
    std::unique_ptr<semantics::DataFlowCollector> dataFlowCollector;

    void SetUp() override {
        SemanticsFixture::SetUp();
        dataFlowRepo = std::make_unique<semantics::DataFlowRepository>();
        dataFlowCollector = std::make_unique<semantics::DataFlowCollector>(
            program,
            context->getPlatform(),
            dataFlowRepo.get());
    }

    ::testing::AssertionResult cmpDataFlow(ircode::Function* function, const std::string& expectedCode) const {
        std::stringstream ss;
        utils::AbstractPrinter printer;
        printer.setOutput(ss);
        for (size_t i = 0; i < 2; ++i)
            printer.startBlock();
        printer.newTabs();
        auto variables = function->getVariables();
        variables.sort([](std::shared_ptr<ircode::Variable> var1, std::shared_ptr<ircode::Variable> var2) {
            return var1->getId() < var2->getId();
        });
        for (auto var : variables) {
            if (auto node = dataFlowRepo->getNode(var)) {
                if (node->predecessors.empty()) {
                    ss << var->getName() << " <- Unknown";
                    printer.newLine();
                } else {
                    for (auto predNode : node->predecessors) {
                        ss << var->getName() << " <- ";
                        if (node->type == semantics::DataFlowNode::Copy)
                            ss << "Copy ";
                        else if (node->type == semantics::DataFlowNode::Write)
                            ss << "Write ";
                        else if (node->type == semantics::DataFlowNode::Read)
                            ss << "Read ";
                        if (predNode->variable) {
                            ss << predNode->variable->getName();
                        } else {
                            if (predNode->type == semantics::DataFlowNode::Start)
                                ss << "<- Start";
                        }
                        if (node->offset > 0) {
                            ss << " + 0x" << utils::to_hex() << node->offset;
                        }
                        printer.newLine();
                    }
                }
            }
        }
        return Compare(ss.str(), expectedCode);
    }
};

TEST_F(DataFlowSemanticsTest, GlobalVarAssignment) {
    auto sourcePCode = "\
        r10:8 = INT_ADD rip:8, 0x10:8 \n\
        STORE r10:8, xmm0:Da \n\
        rax:8 = LOAD r10:8, 4:8 \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[r10]:8 = INT_ADD var1, 0x10:8 \n\
            var3:4 = LOAD xmm0 \n\
            var4[var2]:4 = COPY var3 \n\
            var5:8 = LOAD var2 \n\
            var6[rax]:8 = COPY var5 \
    ";
    auto expectedDataFlow = "\
        var1 <- Copy <- Start \n\
        var2 <- Copy var1 + 0x10 \n\
        var3 <- Unknown \n\
        var4 <- Write var2 \n\
        var4 <- Write var3 \n\
        var5 <- Read var2 \n\
        var6 <- Copy var5 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
}

TEST_F(DataFlowSemanticsTest, GlobalVarAssignmentDouble) {
    auto sourcePCode = "\
        r10:8 = INT_ADD rip:8, 0x10:8 \n\
        STORE r10:8, xmm0:Da \n\
        r10:8 = INT_ADD rip:8, 0x18:8 \n\
        STORE r10:8, xmm1:Da \n\
        rax:8 = LOAD r10:8, 4:8 \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[r10]:8 = INT_ADD var1, 0x10:8 \n\
            var3:4 = LOAD xmm0 \n\
            var4[var2]:4 = COPY var3 \n\
            var5[r10]:8 = INT_ADD var1, 0x18:8 \n\
            var6:4 = LOAD xmm1 \n\
            var7[var5]:4 = COPY var6 \n\
            var8:8 = LOAD var5 \n\
            var9[rax]:8 = COPY var8 \
    ";
    auto expectedDataFlow = "\
        var1 <- Copy <- Start \n\
        var2 <- Copy var1 + 0x10 \n\
        var3 <- Unknown \n\
        var4 <- Write var2 \n\
        var4 <- Write var3 \n\
        var5 <- Copy var1 + 0x18 \n\
        var6 <- Unknown \n\
        var7 <- Write var5 \n\
        var7 <- Write var6 \n\
        var8 <- Read var5 \n\
        var9 <- Copy var8 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
}

TEST_F(DataFlowSemanticsTest, GlobalVarAssignmentObject) {
    auto sourcePCode = "\
        r10:8 = INT_ADD rcx:8, 0x10:8 \n\
        STORE r10:8, xmm1:Da \n\
        r10:8 = INT_ADD rip:8, 0x200:8 \n\
        STORE r10:8, rcx:8 \n\
    ";
    auto expectedIRCode = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rcx \n\
            var2[r10]:8 = INT_ADD var1, 0x10:8 \n\
            var3:4 = LOAD xmm1 \n\
            var4[var2]:4 = COPY var3 \n\
            var5:8 = LOAD rip \n\
            var6[r10]:8 = INT_ADD var5, 0x200:8 \n\
            var7[var6]:8 = COPY var1 \
    ";
    auto expectedDataFlow = "\
        var1 <- Unknown \n\
        var2 <- Copy var1 + 0x10 \n\
        var3 <- Unknown \n\
        var4 <- Write var2 \n\
        var4 <- Write var3 \n\
        var5 <- Copy <- Start \n\
        var6 <- Copy var5 + 0x200 \n\
        var7 <- Write var6 \n\
        var7 <- Write var1 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
}

TEST_F(DataFlowSemanticsTest, GlobalVarAssignmentObjectDouble) {
    auto sourcePCode = "\
        r10:8 = INT_ADD rcx:8, 0x10:8 \n\
        STORE r10:8, xmm1:Da \n\
        r10:8 = INT_ADD rip:8, 0x200:8 \n\
        STORE r10:8, rcx:8 \n\
        r10:8 = INT_ADD rip:8, 0x208:8 \n\
        STORE r10:8, rcx:8 \n\
    ";
    auto expectedIRCode = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rcx \n\
            var2[r10]:8 = INT_ADD var1, 0x10:8 \n\
            var3:4 = LOAD xmm1 \n\
            var4[var2]:4 = COPY var3 \n\
            var5:8 = LOAD rip \n\
            var6[r10]:8 = INT_ADD var5, 0x200:8 \n\
            var7[var6]:8 = COPY var1 \n\
            var8[r10]:8 = INT_ADD var5, 0x208:8 \n\
            var9[var8]:8 = COPY var1 \
    ";
    auto expectedDataFlow = "\
        var1 <- Unknown \n\
        var2 <- Copy var1 + 0x10 \n\
        var3 <- Unknown \n\
        var4 <- Write var2 \n\
        var4 <- Write var3 \n\
        var5 <- Copy <- Start \n\
        var6 <- Copy var5 + 0x200 \n\
        var7 <- Write var6 \n\
        var7 <- Write var1 \n\
        var8 <- Copy var5 + 0x208 \n\
        var9 <- Write var8 \n\
        var9 <- Write var1 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
}

TEST_F(DataFlowSemanticsTest, If) {
    auto sourcePCode = "\
        rax:8 = COPY $10:8 \n\
        $1:1 = INT_NOTEQUAL rcx:4, 5:4 \n\
        CBRANCH <label>, $1:1 \n\
        rax:8 = COPY $11:8 \n\
        <label>: \n\
        r10:8 = INT_2COMP rax:8 \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1, near: B3, far: B4, cond: var4): \n\
            var1:8 = LOAD $U10 \n\
            var2[rax]:8 = COPY var1 \n\
            var3:4 = LOAD rcx \n\
            var4[$U1]:1 = INT_NOTEQUAL var3, 0x5:4 \n\
        Block B3(level: 2, near: B4): \n\
            var5:8 = LOAD $U11 \n\
            var6[rax]:8 = COPY var5 \n\
        Block B4(level: 3): \n\
            var7:8 = REF var2 \n\
            var8:8 = REF var6 \n\
            var9:8 = PHI var7, var8 \n\
            var10[r10]:8 = INT_2COMP var9 \
    ";
    auto expectedDataFlow = "\
        var1 <- Unknown \n\
        var2 <- Copy var1 \n\
        var5 <- Unknown \n\
        var6 <- Copy var5 \n\
        var7 <- Copy var2 \n\
        var8 <- Copy var6 \n\
        var9 <- Copy var7 \n\
        var9 <- Copy var8 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
}
