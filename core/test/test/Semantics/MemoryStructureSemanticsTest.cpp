#include "Test/Core/Semantics/SemanticsFixture.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class MemoryStructureSemanticsTest : public SemanticsFixture
{
protected:
    semantics::SemanticsManager semManager = semantics::SemanticsManager(&program);
    semantics::MemoryStructureSemanticsRepository* memStructRepo = nullptr;

    void SetUp() override {
        IRcodeFixture::SetUp();
        auto memStructRepo = std::make_unique<semantics::MemoryStructureSemanticsRepository>(&semManager);
        this->memStructRepo = memStructRepo.get();
        semManager.addRepository(std::move(memStructRepo));
        semManager.addPropagator(
            std::make_unique<semantics::MemoryStructureSemanticsPropagator>(context->getPlatform(), this->memStructRepo));
    }

    ::testing::AssertionResult cmpSameVariables(ircode::Function* function, const std::string& expectedCode) const {
        auto sortFunc = [](std::shared_ptr<ircode::Variable> var1, std::shared_ptr<ircode::Variable> var2) {
            return var1->getId() < var2->getId();
        };
        std::stringstream ss;
        utils::AbstractPrinter printer;
        printer.setOutput(ss);
        for (size_t i = 0; i < 2; ++i)
            printer.startBlock();
        printer.newTabs();
        auto variables = function->getVariables();
        variables.sort(sortFunc);
        std::set<std::shared_ptr<ircode::Variable>> printedVars;
        for (auto var : variables) {
            if (printedVars.find(var) != printedVars.end())
                continue;
            auto sameVars = memStructRepo->getSameVariables(var);
            sameVars.sort(sortFunc);
            ss << var->getName() << ": ";
            for (auto sameVar : sameVars) {
                ss << sameVar->getName() << " ";
                printedVars.insert(sameVar);
            }
            printer.newLine();
        }
        return Compare(ss.str(), expectedCode);
    }
};

TEST_F(MemoryStructureSemanticsTest, GlobalVarAssignment) {
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
            var4[var1 + 16]:4 = COPY var3 \n\
            var5:8 = LOAD var2 \n\
            var6[rax]:8 = COPY var5 \
    ";
    auto expectedSameVars = "\
        var1: var1 \n\
        var2: var2 \n\
        var3: var3 var4 var5 var6 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpSameVariables(function, expectedSameVars));
}

TEST_F(MemoryStructureSemanticsTest, GlobalVarAssignmentDouble) {
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
            var4[var1 + 16]:4 = COPY var3 \n\
            var5[r10]:8 = INT_ADD var1, 0x18:8 \n\
            var6:4 = LOAD xmm1 \n\
            var7[var1 + 24]:4 = COPY var6 \n\
            var8:8 = LOAD var5 \n\
            var9[rax]:8 = COPY var8 \
    ";
    auto expectedSameVars = "\
        var1: var1 \n\
        var2: var2 \n\
        var3: var3 var4 \n\
        var5: var5 \n\
        var6: var6 var7 var8 var9 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpSameVariables(function, expectedSameVars));
}

TEST_F(MemoryStructureSemanticsTest, GlobalVarAssignmentObject) {
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
            var4[var1 + 16]:4 = COPY var3 \n\
            var5:8 = LOAD rip \n\
            var6[r10]:8 = INT_ADD var5, 0x200:8 \n\
            var7[var5 + 512]:8 = COPY var1 \
    ";
    auto expectedSameVars = "\
        var1: var1 var7 \n\
        var2: var2 \n\
        var3: var3 var4 \n\
        var5: var5 \n\
        var6: var6 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpSameVariables(function, expectedSameVars));
}

TEST_F(MemoryStructureSemanticsTest, GlobalVarAssignmentObjectDouble) {
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
            var4[var1 + 16]:4 = COPY var3 \n\
            var5:8 = LOAD rip \n\
            var6[r10]:8 = INT_ADD var5, 0x200:8 \n\
            var7[var5 + 512]:8 = COPY var1 \n\
            var8[r10]:8 = INT_ADD var5, 0x208:8 \n\
            var9[var5 + 520]:8 = COPY var1 \
    ";
    auto expectedSameVars = "\
        var1: var1 var7 var9 \n\
        var2: var2 \n\
        var3: var3 var4 \n\
        var5: var5 \n\
        var6: var6 \n\
        var8: var8 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpSameVariables(function, expectedSameVars));
}
