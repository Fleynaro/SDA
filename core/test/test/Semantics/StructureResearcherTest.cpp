#include "Test/Core/Semantics/SemanticsFixture.h"
#include "Test/Core/Semantics/DataFlowSemanticsFixture.h"
#include "SDA/Core/Semantics/StructureResearcher.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class StructureResearcherTest : public SemanticsFixture
{
protected:
    std::unique_ptr<semantics::ConstConditionRepository> constCondRepo;
    std::unique_ptr<semantics::DataFlowRepository> dataFlowRepo;
    std::unique_ptr<semantics::DataFlowCollector> dataFlowCollector;
    std::unique_ptr<semantics::StructureRepository> structureRepo;
    std::unique_ptr<semantics::StructureResearcher> structureResearcher;

    void SetUp() override {
        SemanticsFixture::SetUp();
        constCondRepo = std::make_unique<semantics::ConstConditionRepository>(program);
        structureRepo = std::make_unique<semantics::StructureRepository>();
        dataFlowRepo = std::make_unique<semantics::DataFlowRepository>(eventPipe);
        dataFlowCollector = std::make_unique<semantics::DataFlowCollector>(
            program,
            context->getPlatform(),
            dataFlowRepo.get());
        structureResearcher = std::make_unique<semantics::StructureResearcher>(
            program,
            context->getPlatform(),
            structureRepo.get(),
            dataFlowRepo.get(),
            constCondRepo.get());
        eventPipe->connect(constCondRepo->getEventPipe());
        eventPipe->connect(dataFlowCollector->getEventPipe());
        eventPipe->connect(structureResearcher->getEventPipe());
    }

    ::testing::AssertionResult cmpStructures(const std::string& expectedCode) const {
        auto rootStructures = structureRepo->getRootStructures();
        std::list<semantics::Structure*> allStructures;
        for (auto rootStructure : sortByName(rootStructures)) {
            gatherAllChildStructures(rootStructure, allStructures);
        }
        std::stringstream ss;
        bool isFirstPrinted = false;
        for (auto structure : allStructures) {
            if (structure->fields.empty())
                continue;
            if (isFirstPrinted) {
                ss << std::endl << std::endl;
            } else {
                isFirstPrinted = true;
            }
            printStructure(ss, structure);
        }
        return Compare(ss.str(), expectedCode);
    }

    void gatherAllChildStructures(semantics::Structure* rootStructure, std::list<semantics::Structure*>& result) const {
        std::list<semantics::Structure*> structuresToVisit;
        structuresToVisit.push_back(rootStructure);
        while (!structuresToVisit.empty()) {
            auto structure = structuresToVisit.front();
            structuresToVisit.pop_front();
            result.push_back(structure);
            for (auto child : sortByName(structure->childs)) {
                if (std::find(result.begin(), result.end(), child) != result.end()) {
                    continue;
                }
                structuresToVisit.push_back(child);
            }
        }
    }

    void printStructure(std::stringstream& ss, semantics::Structure* structure) const {
        ss << "struct " << structure->name << " ";
        if (!structure->parents.empty()) {
            ss << ": ";
            auto parents = sortByName(structure->parents);
            for (auto parent : parents) {
                ss << parent->name;
                if (parent != parents.back()) {
                    ss << ", ";
                }
            }
            ss << " ";
        }
        ss << "{" << std::endl;
        for (auto& [offset, fieldStructure] : structure->fields) {
            auto offsetStr = (std::stringstream() << "0x" << utils::to_hex() << offset).str();
            std::string valueStructures;
            for (auto child : sortByName(fieldStructure->childs)) {
                valueStructures += child->name + ", ";
            }
            if (!valueStructures.empty()) {
                valueStructures.pop_back();
                valueStructures.pop_back();
            }
            ss << "    " << offsetStr << ": " << valueStructures << std::endl;
        }
        ss << "}";
    }

    std::list<semantics::Structure*> sortByName(std::list<semantics::Structure*> structures) const {
        structures.sort([](semantics::Structure* a, semantics::Structure* b) {
            return a->name < b->name;
        });
        return structures;
    }

    ::testing::AssertionResult cmpDataFlow(ircode::Function* function, const std::string& expectedCode) const {
       return DataFlowSemanticsFixture::CmpDataFlow(dataFlowRepo.get(), function, expectedCode);
    }
};

TEST_F(StructureResearcherTest, GlobalVarAssignment) {
    /*
        float func(float param1) {
            globalVar_0x10 = param1;
            return globalVar_0x10;
        }
    */
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
    auto expectedStructures = "\
        struct root { \n\
            0x10: B0:var3, B0:var5 \n\
        } \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
    ASSERT_TRUE(cmpStructures(expectedStructures));
}

TEST_F(StructureResearcherTest, GlobalVarAssignmentDouble) {
    /*
        float func(float param1, float param2) {
            globalVar_0x10 = param1;
            globalVar_0x18 = param2;
            return globalVar_0x18;
        }
    */
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
    auto expectedStructures = "\
        struct root { \n\
            0x10: B0:var3 \n\
            0x18: B0:var6, B0:var8 \n\
        } \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
    ASSERT_TRUE(cmpStructures(expectedStructures));
}

TEST_F(StructureResearcherTest, GlobalVarAssignmentObject) {
    /*
        void func(Object* param1, float param2) {
            param1->field_0x10 = param2;
            globalVar_0x200 = param1;
        }
    */
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
    auto expectedStructures = "\
        struct root { \n\
            0x200: B0:var1 \n\
        } \n\
        \n\
        struct B0:var1 : root_0x200 { \n\
            0x10: B0:var3 \n\
        } \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
    ASSERT_TRUE(cmpStructures(expectedStructures));
}

TEST_F(StructureResearcherTest, If) {
    /*
        void func(Object* param1) {
            if (param1->field_0x0 == 1) {
                player = static_cast<Player*>(param1);
                player->field_0x10 = 100;
            } else if (param1->field_0x0 == 2) {
                vehicle = static_cast<Vehicle*>(param1);
                vehicle->field_0x18 = 200;
            }
        }
    */
    auto sourcePCode = "\
        $1:4 = LOAD rcx:8, 4:8 \n\
        $2:1 = INT_NOTEQUAL $1:4, 1:4 \n\
        CBRANCH <label1>, $2:1 \n\
        $3:8 = INT_ADD rcx:8, 0x10:8 \n\
        STORE $3:8, 100:8 \n\
        <label1>: \n\
        $2:1 = INT_NOTEQUAL $1:4, 2:4 \n\
        CBRANCH <label2>, $2:1 \n\
        $3:8 = INT_ADD rcx:8, 0x18:8 \n\
        STORE $3:8, 200:8 \n\
        <label2>: \n\
        RETURN \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1, near: B3, far: B5, cond: var4): \n\
            var1:8 = LOAD rcx \n\
            var2:4 = LOAD var1 \n\
            var3[$U1]:4 = COPY var2 \n\
            var4[$U2]:1 = INT_NOTEQUAL var3, 0x1:4 \n\
        Block B3(level: 2, near: B5): \n\
            var5:8 = REF var1 \n\
            var6[$U3]:8 = INT_ADD var5, 0x10:8 \n\
            var7[var6]:8 = COPY 0x64:8 \n\
        Block B5(level: 3, near: B7, far: B9, cond: var9): \n\
            var8:4 = REF var3 \n\
            var9[$U2]:1 = INT_NOTEQUAL var8, 0x2:4 \n\
        Block B7(level: 4, near: B9): \n\
            var10:8 = REF var1 \n\
            var11[$U3]:8 = INT_ADD var10, 0x18:8 \n\
            var12[var11]:8 = COPY 0xc8:8 \n\
        Block B9(level: 5): \n\
            empty \
    ";
    auto expectedDataFlow = "\
        var1 <- Unknown \n\
        var2 <- Read var1 \n\
        var3 <- Copy var2 \n\
        var5 <- Copy var1 \n\
        var6 <- Copy var5 + 0x10 \n\
        var8 <- Copy var3 \n\
        var10 <- Copy var1 \n\
        var11 <- Copy var10 + 0x18 \
    ";
    auto expectedStructures = "\
        \n\
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
    ASSERT_TRUE(cmpStructures(expectedStructures));
}
