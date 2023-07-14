#include "Test/Core/Semantics/SemanticsFixture.h"
#include "Test/Core/Semantics/DataFlowSemanticsFixture.h"
#include "Test/Core/Semantics/ConstConditionSemanticsFixture.h"
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
        structureRepo = std::make_unique<semantics::StructureRepository>(eventPipe);
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
        std::list<semantics::Structure*> allStructures;
        auto rootStructures = structureRepo->getRootStructures();
        for (auto rootStructure : sortByName(rootStructures)) {
            gatherAllChildStructures(rootStructure, allStructures);
        }
        for (auto structure : sortByName(structureRepo->getAllStructures())) {
            if (std::find(allStructures.begin(), allStructures.end(), structure) != allStructures.end()) {
                continue;
            }
            allStructures.push_back(structure);
        }
        std::stringstream ss;
        bool isFirstPrinted = false;
        for (auto structure : allStructures) {
            if (structure->fields.empty() && structure->conditions.values().empty())
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
        std::set<size_t> fieldOffsets;
        for (auto& [offset, _] : structure->fields) {
            fieldOffsets.insert(offset);
        }
        for (auto& [offset, _] : structure->conditions.values()) {
            fieldOffsets.insert(offset);
        }
        for (auto offset : fieldOffsets) {
            std::string sep;
            std::stringstream fieldValues;
            // structures
            auto it = structure->fields.find(offset);
            if (it != structure->fields.end()) {      
                auto childs = sortByName(it->second->childs);
                for (auto child : childs) {
                    fieldValues << sep << child->name;
                    sep = ", ";
                }
            }
            // constants
            auto it2 = structure->conditions.values().find(offset);
            if (it2 != structure->conditions.values().end()) {
                for (auto value : it2->second) {
                    fieldValues << sep << "0x" << utils::to_hex() << value;
                    sep = ", ";
                }
            }
            auto offsetStr = (std::stringstream() << "0x" << utils::to_hex() << offset).str();
            ss << "    " << offsetStr << ": " << fieldValues.str() << std::endl;
        }
        ss << "}";
    }

    std::list<semantics::Structure*> sortByName(std::list<semantics::Structure*> structures) const {
        structures.sort([](semantics::Structure* a, semantics::Structure* b) {
            return a->name < b->name;
        });
        return structures;
    }

    ::testing::AssertionResult cmpConditions(ircode::Function* function, const std::string& expectedCode) const {
        return ConstConditionSemanticsFixture::CmpConditions(constCondRepo.get(), function, expectedCode);
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
        var1 <- Copy Start \n\
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
        var1 <- Copy Start \n\
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
        var5 <- Copy Start \n\
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

TEST_F(StructureResearcherTest, DoubleRead) {
    /*
        void func() {
            globalVar_0x10 = globalVar_0x100->field_0x10;
        }
    */
    auto sourcePCode = "\
        $1:8 = INT_ADD rip:8, 0x100:8 \n\
        $2:8 = LOAD $1:8, 8:8 \n\
        $3:8 = INT_ADD $2:8, 0x10:8 \n\
        $4:4 = LOAD $3:8, 4:8 \n\
        $5:8 = INT_ADD rip:8, 0x10:8 \n\
        STORE $5:8, $4:4 \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[$U1]:8 = INT_ADD var1, 0x100:8 \n\
            var3:8 = LOAD var2 \n\
            var4[$U2]:8 = COPY var3 \n\
            var5[$U3]:8 = INT_ADD var4, 0x10:8 \n\
            var6:4 = LOAD var5 \n\
            var7[$U4]:4 = COPY var6 \n\
            var8[$U5]:8 = INT_ADD var1, 0x10:8 \n\
            var9[var8]:4 = COPY var7 \
    ";
    auto expectedDataFlow = "\
        var1 <- Copy Start \n\
        var2 <- Copy var1 + 0x100 \n\
        var3 <- Read var2 \n\
        var4 <- Copy var3 \n\
        var5 <- Copy var3 + 0x10 \n\
        var6 <- Read var5 \n\
        var7 <- Copy var6 \n\
        var8 <- Copy var1 + 0x10 \n\
        var9 <- Write var8 \n\
        var9 <- Write var7 \
    ";
    auto expectedStructures = "\
        struct root { \n\
            0x10: B0:var6 \n\
            0x100: B0:var3 \n\
        } \n\
        \n\
        struct B0:var3 : root_0x100 { \n\
            0x10: B0:var6 \n\
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
                player->field_0x10 = 0x64;
            } else if (param1->field_0x0 == 2) {
                vehicle = static_cast<Vehicle*>(param1);
                vehicle->field_0x18 = 0xC8;
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
            var11:8 = REF var5 \n\
            var12[$U3]:8 = INT_ADD var10, 0x18:8 \n\
            var13[var12]:8 = COPY 0xc8:8 \n\
        Block B9(level: 5): \n\
            empty \
    ";
    auto expectedConditions = "\
        Block B3: \n\
            var3 == 1 \n\
        Block B7: \n\
            var8 == 2 \
    ";
    auto expectedDataFlow = "\
        var1 <- Unknown \n\
        var2 <- Read var1 \n\
        var3 <- Copy var2 \n\
        var5 <- Copy var1 \n\
        var6 <- Copy var5 + 0x10 \n\
        var7 <- Write var6 \n\
        var7 <- Write 0x64 \n\
        var8 <- Copy var3 \n\
        var10 <- Copy var1 \n\
        var11 <- Copy var5 \n\
        var12 <- Copy var10 + 0x18 \n\
        var13 <- Write var12 \n\
        var13 <- Write 0xc8 \
    ";
    auto expectedStructures = "\
        struct B0:var1 { \n\
            0x0: B0:var2 \n\
        } \n\
        \n\
        struct B0:var10 : B0:var1 { \n\
            0x0: 0x2 \n\
            0x18: 0xc8 \n\
        } \n\
        \n\
        struct B0:var5 : B0:var1 { \n\
            0x0: 0x1 \n\
            0x10: 0x64 \n\
        } \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpConditions(function, expectedConditions));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
    ASSERT_TRUE(cmpStructures(expectedStructures));
}

TEST_F(StructureResearcherTest, IfRewrite) {
    /*
        void func(Object* param1) {
            if (param1->field_0x0 == 1) {
                param1->field_0x0 = 2; // rewrite
            }
        }
    */
    auto sourcePCode = "\
        $1:4 = LOAD rcx:8, 4:8 \n\
        $2:1 = INT_NOTEQUAL $1:4, 1:4 \n\
        CBRANCH <label>, $2:1 \n\
        STORE rcx:8, 2:4 \n\
        <label>: \n\
        RETURN \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1, near: B3, far: B4, cond: var4): \n\
            var1:8 = LOAD rcx \n\
            var2:4 = LOAD var1 \n\
            var3[$U1]:4 = COPY var2 \n\
            var4[$U2]:1 = INT_NOTEQUAL var3, 0x1:4 \n\
        Block B3(level: 2, near: B4): \n\
            var5:8 = REF var1 \n\
            var6[var5]:4 = COPY 0x2:4 \n\
        Block B4(level: 3): \n\
            empty \
    ";
    auto expectedConditions = "\
        Block B3: \n\
            var3 == 1 \
    ";
    auto expectedDataFlow = "\
        var1 <- Unknown \n\
        var2 <- Read var1 \n\
        var3 <- Copy var2 \n\
        var5 <- Copy var1 \n\
        var6 <- Write var5 \n\
        var6 <- Write 0x2 \
    ";
    auto expectedStructures = "\
        struct B0:var1 { \n\
            0x0: B0:var2 \n\
        } \n\
        \n\
        struct B0:var5 : B0:var1 { \n\
            0x0: 0x2 \n\
        } \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpConditions(function, expectedConditions));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
    ASSERT_TRUE(cmpStructures(expectedStructures));
}

TEST_F(StructureResearcherTest, IfNested) {
    /*
        void func(Object* param1) {
            if (param1->field_0x0 == 1) {
                player = static_cast<Player*>(param1);
                player->field_0x20 = 0x5;
                if (player->field_0x40 == 2) {
                    player->field_0x10 = 0x64;
                }
            }
        }
    */
   auto sourcePCode = "\
        $1:4 = LOAD rcx:8, 4:8 \n\
        $2:1 = INT_NOTEQUAL $1:4, 1:4 \n\
        CBRANCH <label>, $2:1 \n\
        $3:8 = INT_ADD rcx:8, 0x20:8 \n\
        STORE $3:8, 5:4 \n\
        $4:4 = INT_ADD rcx:8, 0x40:8 \n\
        $5:4 = LOAD $4:4, 4:4 \n\
        $6:1 = INT_NOTEQUAL $5:4, 2:4 \n\
        CBRANCH <label>, $6:1 \n\
        $7:4 = INT_ADD rcx:8, 0x10:8 \n\
        STORE $7:4, 100:4 \n\
        <label>: \n\
        RETURN \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1, near: B3, far: Bb, cond: var4): \n\
            var1:8 = LOAD rcx \n\
            var2:4 = LOAD var1 \n\
            var3[$U1]:4 = COPY var2 \n\
            var4[$U2]:1 = INT_NOTEQUAL var3, 0x1:4 \n\
        Block B3(level: 2, near: B9, far: Bb, cond: var11): \n\
            var5:8 = REF var1 \n\
            var6[$U3]:8 = INT_ADD var5, 0x20:8 \n\
            var7[var6]:4 = COPY 0x5:4 \n\
            var8[$U4]:4 = INT_ADD var5, 0x40:8 \n\
            var9:4 = LOAD var8 \n\
            var10[$U5]:4 = COPY var9 \n\
            var11[$U6]:1 = INT_NOTEQUAL var10, 0x2:4 \n\
        Block B9(level: 3, near: Bb): \n\
            var12:8 = REF var5 \n\
            var13[$U7]:4 = INT_ADD var12, 0x10:8 \n\
            var14[var13]:4 = COPY 0x64:4 \n\
        Block Bb(level: 4): \n\
            empty \
    ";
    auto expectedConditions = "\
        Block B3: \n\
            var3 == 1 \n\
        Block B9: \n\
            var3 == 1 \n\
            var10 == 2 \
    ";
    auto expectedDataFlow = "\
        var1 <- Unknown \n\
        var2 <- Read var1 \n\
        var3 <- Copy var2 \n\
        var5 <- Copy var1 \n\
        var6 <- Copy var5 + 0x20 \n\
        var7 <- Write var6 \n\
        var7 <- Write 0x5 \n\
        var8 <- Copy var5 + 0x40 \n\
        var9 <- Read var8 \n\
        var10 <- Copy var9 \n\
        var12 <- Copy var5\n\
        var13 <- Copy var12 + 0x10 \n\
        var14 <- Write var13 \n\
        var14 <- Write 0x64 \
    ";
    auto expectedStructures = "\
        struct B0:var1 { \n\
            0x0: B0:var2 \n\
        } \n\
        \n\
        struct B0:var5 : B0:var1 { \n\
            0x0: 0x1 \n\
            0x20: 0x5 \n\
            0x40: B0:var9 \n\
        } \n\
        \n\
        struct B0:var12 : B0:var5 { \n\
            0x10: 0x64 \n\
            0x40: 0x2 \n\
        } \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpConditions(function, expectedConditions));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
    ASSERT_TRUE(cmpStructures(expectedStructures));
}

TEST_F(StructureResearcherTest, LoopObjectArray) {
    /*
        void func(Object* param1) {
            int i = 0;
            while (i != 3) {
                param1->field_0x10 = 0x64;
                param1 += 0x1; // sizeof(Object) = 0x20
                i++;
            }
        }
    */
    auto sourcePCode = "\
        $1:8 = COPY rcx:8 \n\
        STORE $1:8, 1:4 \n\
        $2:4 = COPY 0x0:4 \n\
        <loop_cond>: \n\
        $3:1 = INT_EQUAL $2:4, 0x3:4 \n\
        CBRANCH <loop_end>, $3:1 \n\
        $4:8 = INT_ADD $1:8, 0x10:8 \n\
        STORE $4:8, 0x64:8 \n\
        $1:8 = INT_ADD $1:8, 0x20:8 \n\
        $2:4 = INT_ADD $2:4, 0x1:4 \n\
        BRANCH <loop_cond> \n\
        <loop_end>: \n\
        RETURN \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1, near: B3): \n\
            var1:8 = LOAD rcx \n\
            var2[$U1]:8 = COPY var1 \n\
            var3[var2]:4 = COPY 0x1:4 \n\
            var4[$U2]:4 = COPY 0x0:4 \n\
        Block B3(level: 2, near: B5, far: Ba, cond: var16): \n\
            var5:4 = REF var4 \n\
            var6:4 = REF var14 \n\
            var15:4 = PHI var5, var6 \n\
            var16[$U3]:1 = INT_EQUAL var15, 0x3:4 \n\
        Block B5(level: 3, far: B3): \n\
            var7:8 = REF var2 \n\
            var8:8 = REF var12 \n\
            var9:8 = PHI var7, var8 \n\
            var10[$U4]:8 = INT_ADD var9, 0x10:8 \n\
            var11[var10]:8 = COPY 0x64:8 \n\
            var12[$U1]:8 = INT_ADD var9, 0x20:8 \n\
            var13:4 = REF var15 \n\
            var14[$U2]:4 = INT_ADD var13, 0x1:4 \n\
        Block Ba(level: 3): \n\
            empty \
    ";
    auto expectedConditions = "\
        Block B5: \n\
            var15 != 3 \n\
        Block Ba: \n\
            var15 == 3 \
    ";
    auto expectedDataFlow = "\
        var1 <- Unknown \n\
        var2 <- Copy var1 \n\
        var3 <- Write var2 \n\
        var3 <- Write 0x1 \n\
        var4 <- Copy 0x0 \n\
        var5 <- Copy var4 \n\
        var6 <- Copy var14 \n\
        var7 <- Copy var2 \n\
        var8 <- Copy var12 \n\
        var9 <- Copy var7 \n\
        var9 <- Copy var8 \n\
        var10 <- Copy var9 + 0x10 \n\
        var11 <- Write var10 \n\
        var11 <- Write 0x64 \n\
        var12 <- Copy var9 + 0x20 \n\
        var13 <- Copy var15 \n\
        var14 <- Unknown \n\
        var15 <- Copy var5 \n\
        var15 <- Copy var6 \
    ";
    auto expectedStructures = "\
        struct B0:var1 : B0:var9 { \n\
            0x0: 0x1 \n\
        } \n\
        \n\
        struct B0:var9 : B0:var9 { \n\
            0x10: 0x64 \n\
        } \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpConditions(function, expectedConditions));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
    ASSERT_TRUE(cmpStructures(expectedStructures));
}

TEST_F(StructureResearcherTest, Functions) {
    /*
        void main() {
            if (globalVar_0x100->field_0x0 == 1) {
                player = static_cast<Player*>(globalVar_0x100);
                func(player);
            }
            else if (globalVar_0x100->field_0x0 == 2) {
                vehicle = static_cast<Vehicle*>(globalVar_0x100);
                func(vehicle);
            }
        }

        void func(Object* param1) {
            param1->field_0x4 = 1;
            if (param1->field_0x0 == 1) {
                player = static_cast<Player*>(param1);
                player->field_0x10 = 0x64;
            }
        }
    */
   auto sourcePCode = "\
        // main() \n\
        $1:8 = INT_ADD rip:8, 0x100:8 \n\
        $2:8 = LOAD $1:8, 8:8 \n\
        $3:4 = LOAD $2:8, 4:8 \n\
        $4:1 = INT_NOTEQUAL $3:4, 1:4 \n\
        CBRANCH <main_label>, $4:1 \n\
        rcx:8 = COPY $2:8 \n\
        CALL <func> \n\
        BRANCH <main_end> \n\
        <main_label>: \n\
        $5:1 = INT_NOTEQUAL $3:4, 2:4 \n\
        CBRANCH <main_end>, $5:1 \n\
        rcx:8 = COPY $2:8 \n\
        CALL <func> \n\
        <main_end>: \n\
        RETURN \n\
        \n\
        \n\
        // func(Object* param1) \n\
        <func>: \n\
        $1:4 = LOAD rcx:8, 4:8 \n\
        $2:8 = INT_ADD rcx:8, 0x4:8 \n\
        STORE $2:8, 1:4 \n\
        $3:1 = INT_NOTEQUAL $1:4, 1:4 \n\
        CBRANCH <func_end>, $3:1 \n\
        $4:8 = INT_ADD rcx:8, 0x10:8 \n\
        STORE $4:8, 0x64:4 \n\
        <func_end>: \n\
        RETURN \
    ";
    auto funcSig = "\
        funcSig = signature fastcall bool(void* param1) \
    ";
    auto expectedIRCodeOfMainFunc = "\
        Block B0(level: 1, near: B5, far: B8, cond: var7): \n\
            var1:8 = LOAD rip \n\
            var2[$U1]:8 = INT_ADD var1, 0x100:8 \n\
            var3:8 = LOAD var2 \n\
            var4[$U2]:8 = COPY var3 \n\
            var5:4 = LOAD var4 \n\
            var6[$U3]:4 = COPY var5 \n\
            var7[$U4]:1 = INT_NOTEQUAL var6, 0x1:4 \n\
        Block B5(level: 2, far: Bc): \n\
            var8:8 = REF var4 \n\
            var9[rcx]:8 = COPY var8 \n\
            var10[rax]:1 = CALL 0xd00:8, var9 \n\
        Block B8(level: 2, near: Ba, far: Bc, cond: var12): \n\
            var11:4 = REF var6 \n\
            var12[$U5]:1 = INT_NOTEQUAL var11, 0x2:4 \n\
        Block Ba(level: 3, near: Bc): \n\
            var13:8 = REF var4 \n\
            var14[rcx]:8 = COPY var13 \n\
            var15[rax]:1 = CALL 0xd00:8, var14 \n\
        Block Bc(level: 4): \n\
            empty \
    ";
    auto expectedIRCodeOfFunc = "\
        Block Bd(level: 1, near: B12, far: B14, cond: var6): \n\
            var1:8 = LOAD rcx // param1 \n\
            var2:4 = LOAD var1 \n\
            var3[$U1]:4 = COPY var2 \n\
            var4[$U2]:8 = INT_ADD var1, 0x4:8 \n\
            var5[var4]:4 = COPY 0x1:4 \n\
            var6[$U3]:1 = INT_NOTEQUAL var3, 0x1:4 \n\
        Block B12(level: 2, near: B14): \n\
            var7:8 = REF var1 \n\
            var8[$U4]:8 = INT_ADD var7, 0x10:8 \n\
            var9[var8]:4 = COPY 0x64:4 \n\
        Block B14(level: 3): \n\
            empty \
    ";
    auto expectedConditionsOfMainFunc = "\
        Block B5: \n\
            var6 == 1 \n\
        Block B8: \n\
            var6 != 1 \n\
        Block Ba: \n\
            var6 != 1 \n\
            var11 == 2 \
    ";
    auto expectedConditionsOfFunc = "\
        Block B12: \n\
            var3 == 1 \
    ";
    auto expectedDataFlowOfMainFunc = "\
        var1 <- Copy Start \n\
        var2 <- Copy var1 + 0x100 \n\
        var3 <- Read var2 \n\
        var4 <- Copy var3 \n\
        var5 <- Read var4 \n\
        var6 <- Copy var5 \n\
        var8 <- Copy var4 \n\
        var9 <- Copy var8 \n\
        var11 <- Copy var6 \n\
        var13 <- Copy var4 \n\
        var14 <- Copy var13 \n\
        Bd:var1 <- Copy var9 \n\
        Bd:var1 <- Copy var14 \
    ";
    auto expectedDataFlowOfFunc = "\
        var1 <- Copy B0:var9 \n\
        var1 <- Copy B0:var14 \n\
        var2 <- Read var1 \n\
        var3 <- Copy var2 \n\
        var4 <- Copy var1 + 0x4 \n\
        var5 <- Write var4 \n\
        var5 <- Write 0x1 \n\
        var7 <- Copy var1 \n\
        var8 <- Copy var7 + 0x10 \n\
        var9 <- Write var8 \n\
        var9 <- Write 0x64 \
    ";
    auto expectedStructures = "\
        struct Bd:var1 { \n\
            0x0: Bd:var2 \n\
            0x4: 0x1 \n\
        } \n\
        \n\
        struct B0:var13 : B0:var3, Bd:var1 { \n\
            0x0: 0x2 \n\
        } \n\
        \n\
        struct B0:var8 : B0:var3, Bd:var1 { \n\
            0x0: 0x1 \n\
        } \n\
        \n\
        struct Bd:var7 : Bd:var1 { \n\
            0x0: 0x1 \n\
            0x10: 0x64 \n\
        } \n\
        \n\
        struct root { \n\
            0x100: B0:var3 \n\
        } \n\
        \n\
        struct B0:var3 : root_0x100 { \n\
            0x0: B0:var5 \n\
        } \
    ";
    auto mainFunction = parsePcode(sourcePCode, program);
    auto funcSigDt = dynamic_cast<SignatureDataType*>(
        parseDataType(funcSig));
    auto func = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(13, 0)));
    func->getFunctionSymbol()->getSignature()->copyFrom(funcSigDt);
    ASSERT_TRUE(cmp(mainFunction, expectedIRCodeOfMainFunc));
    ASSERT_TRUE(cmp(func, expectedIRCodeOfFunc));
    ASSERT_TRUE(cmpConditions(mainFunction, expectedConditionsOfMainFunc));
    ASSERT_TRUE(cmpConditions(func, expectedConditionsOfFunc));
    ASSERT_TRUE(cmpDataFlow(mainFunction, expectedDataFlowOfMainFunc));
    ASSERT_TRUE(cmpDataFlow(func, expectedDataFlowOfFunc));
    ASSERT_TRUE(cmpStructures(expectedStructures));
}

TEST_F(StructureResearcherTest, ReturnedObjectAsArgument) {
    /*
        void main() {
            func2(func1())
        }

        Object* func1() {
            return globalVar_0x100;
        }

        void func2(Object* param1) {
            param1->field_0x4 = 1;
        }
    */
   auto sourcePCode = "\
        // main() \n\
        CALL <func1> \n\
        rcx:8 = COPY rax:8 \n\
        CALL <func2> \n\
        RETURN \n\
        \n\
        \n\
        // Object* func1() \n\
        <func1>: \n\
        $1:8 = INT_ADD rip:8, 0x100:8 \n\
        rax:8 = LOAD $1:8, 0x8:8 \n\
        RETURN \n\
        \n\
        \n\
        // void func2(Object* param1) \n\
        <func2>: \n\
        $1:8 = INT_ADD rcx:8, 0x4:8 \n\
        STORE $1:8, 0x1:4 \n\
        RETURN \
    ";
    auto func1Sig = "\
        func1Sig = signature fastcall void*() \
    ";
    auto func2Sig = "\
        func2Sig = signature fastcall void(void* param1) \
    ";
    auto expectedIRCodeOfMainFunc = "\
        Block B0(level: 1): \n\
            var1[rax]:8 = CALL 0x400:8 \n\
            var2[rcx]:8 = COPY var1 \n\
            var3:1 = CALL 0x700:8, var2 \
    ";
    auto expectedIRCodeOfFunc1 = "\
        Block B4(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[$U1]:8 = INT_ADD var1, 0x100:8 \n\
            var3:8 = LOAD var2 \n\
            var4[rax]:8 = COPY var3 // return \
    ";
    auto expectedIRCodeOfFunc2 = "\
        Block B7(level: 1): \n\
            var1:8 = LOAD rcx // param1 \n\
            var2[$U1]:8 = INT_ADD var1, 0x4:8 \n\
            var3[var2]:4 = COPY 0x1:4 \
    ";
    auto expectedDataFlowOfMainFunc = "\
        var1 <- Copy B4:var4 \n\
        var2 <- Copy var1 \n\
        B7:var1 <- Copy var2 \
    ";
    auto expectedDataFlowOfFunc1 = "\
        var1 <- Copy Start \n\
        var2 <- Copy var1 + 0x100 \n\
        var3 <- Read var2 \n\
        var4 <- Copy var3 \n\
        B0:var1 <- Copy var4 \
    ";
    auto expectedDataFlowOfFunc2 = "\
        var1 <- Copy B0:var2 \n\
        var2 <- Copy var1 + 0x4 \n\
        var3 <- Write var2 \n\
        var3 <- Write 0x1 \
    ";
    auto expectedStructures = "\
        struct root { \n\
            0x100: B4:var3 \n\
        } \n\
        \n\
        struct B4:var3 : root_0x100 { \n\
            0x4: 0x1 \n\
        } \
    ";
    auto mainFunction = parsePcode(sourcePCode, program);
    auto func1SigDt = dynamic_cast<SignatureDataType*>(
        parseDataType(func1Sig));
    auto func2SigDt = dynamic_cast<SignatureDataType*>(
        parseDataType(func2Sig));
    auto func1 = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(4, 0)));
    auto func2 = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(7, 0)));
    func1->getFunctionSymbol()->getSignature()->copyFrom(func1SigDt);
    func2->getFunctionSymbol()->getSignature()->copyFrom(func2SigDt);
    ASSERT_TRUE(cmp(mainFunction, expectedIRCodeOfMainFunc));
    ASSERT_TRUE(cmp(func1, expectedIRCodeOfFunc1));
    ASSERT_TRUE(cmp(func2, expectedIRCodeOfFunc2));
    ASSERT_TRUE(cmpDataFlow(mainFunction, expectedDataFlowOfMainFunc));
    ASSERT_TRUE(cmpDataFlow(func1, expectedDataFlowOfFunc1));
    ASSERT_TRUE(cmpDataFlow(func2, expectedDataFlowOfFunc2));
    ASSERT_TRUE(cmpStructures(expectedStructures));
}

TEST_F(StructureResearcherTest, TwoSameFunctionsCall) {
    /*
        void main(int param1) {
            if (param1 == 1) {
                object = func();
                object->field_0x10 = 1;
            }
            else {
                object = func();
            }
            object->field_0x20 = 2;
        }

        void func() {
            return globalVar_0x100;
        }
    */
   auto sourcePCode = "\
        // main(int param1) \n\
        $1:1 = INT_NOTEQUAL rcx:4, 1:4 \n\
        CBRANCH <else>, $1:1 \n\
        CALL <func> \n\
        $2:8 = INT_ADD rax:8, 0x10:8 \n\
        STORE $2:8, 1:4 \n\
        BRANCH <main_end> \n\
        <else>: \n\
        CALL <func> \n\
        <main_end>: \n\
        $3:8 = INT_ADD rax:8, 0x20:8 \n\
        STORE $3:8, 2:4 \n\
        RETURN \n\
        \n\
        \n\
        // void* func() \n\
        <func>: \n\
        $1:8 = INT_ADD rip:8, 0x100:8 \n\
        rax:8 = LOAD $1:8 \n\
        RETURN \
    ";
    auto funcSig = "\
        funcSig = signature fastcall void*() \
    ";
    auto expectedIRCodeOfMainFunc = "\
        Block B0(level: 1, near: B2, far: B6, cond: var2): \n\
            var1:4 = LOAD rcx \n\
            var2[$U1]:1 = INT_NOTEQUAL var1, 0x1:4 \n\
        Block B2(level: 2, far: B7): \n\
            var3[rax]:8 = CALL 0xa00:8 \n\
            var4[$U2]:8 = INT_ADD var3, 0x10:8 \n\
            var5[var4]:4 = COPY 0x1:4 \n\
        Block B6(level: 2, near: B7): \n\
            var6[rax]:8 = CALL 0xa00:8 \n\
        Block B7(level: 3): \n\
            var7:8 = REF var3 \n\
            var8:8 = REF var6 \n\
            var9:8 = PHI var7, var8 \n\
            var10[$U3]:8 = INT_ADD var9, 0x20:8 \n\
            var11[var10]:4 = COPY 0x2:4 \
    ";
    auto expectedIRCodeOfFunc = "\
        Block Ba(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[$U1]:8 = INT_ADD var1, 0x100:8 \n\
            var3:8 = LOAD var2 \n\
            var4[rax]:8 = COPY var3 // return \
    ";
    auto expectedConditionsOfMainFunc = "\
        Block B2: \n\
            var1 == 1 \n\
        Block B6: \n\
            var1 != 1 \
    ";
    auto expectedDataFlowOfMainFunc = "\
        var3 <- Copy Ba:var4 \n\
        var4 <- Copy var3 + 0x10 \n\
        var5 <- Write var4 \n\
        var5 <- Write 0x1 \n\
        var6 <- Copy Ba:var4 \n\
        var7 <- Copy var3 \n\
        var8 <- Copy var6 \n\
        var9 <- Copy var7 \n\
        var9 <- Copy var8 \n\
        var10 <- Copy var9 + 0x20 \n\
        var11 <- Write var10 \n\
        var11 <- Write 0x2 \
    ";
    auto expectedDataFlowOfFunc = "\
        var1 <- Copy Start \n\
        var2 <- Copy var1 + 0x100 \n\
        var3 <- Read var2 \n\
        var4 <- Copy var3 \n\
        B0:var3 <- Copy var4 \n\
        B0:var6 <- Copy var4 \
    ";
    auto expectedStructures = "\
        struct B0:var9 { \n\
            0x20: 0x2 \n\
        } \n\
        \n\
        struct Ba:var3 : B0:var9, root_0x100 { \n\
            0x10: 0x1 \n\
        } \n\
        \n\
        struct root { \n\
            0x100: Ba:var3 \n\
        } \
    ";
    auto mainFunction = parsePcode(sourcePCode, program);
    auto funcSigDt = dynamic_cast<SignatureDataType*>(
        parseDataType(funcSig));
    auto func = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(10, 0)));
    func->getFunctionSymbol()->getSignature()->copyFrom(funcSigDt);
    ASSERT_TRUE(cmp(mainFunction, expectedIRCodeOfMainFunc));
    ASSERT_TRUE(cmp(func, expectedIRCodeOfFunc));
    ASSERT_TRUE(cmpConditions(mainFunction, expectedConditionsOfMainFunc));
    ASSERT_TRUE(cmpDataFlow(mainFunction, expectedDataFlowOfMainFunc));
    ASSERT_TRUE(cmpDataFlow(func, expectedDataFlowOfFunc));
    ASSERT_TRUE(cmpStructures(expectedStructures));
}

TEST_F(StructureResearcherTest, NewFunctionAdded) {
    /*
        void func1() {
            globalVar_0x100->field_0x0 = 0x1;
            func3(globalVar_0x100);
        }

        void func2() {
            globalVar_0x200->field_0x0 = 0x2;
            func3(globalVar_0x200);
        }

        void func3(Object* param1) {
            param1->field_0x10 = globalVar_0x20;
            param1->field_0x20 = 0x5;
        }
    */
   auto sourcePCode = "\
        // func1() \n\
        $1:8 = INT_ADD rip:8, 0x100:8 \n\
        rcx:8 = LOAD $1:8 \n\
        STORE rcx:8, 0x1:4 \n\
        CALL <func3> \n\
        RETURN \n\
        \n\
        \n\
        // func2() \n\
        $1:8 = INT_ADD rip:8, 0x200:8 \n\
        rcx:8 = LOAD $1:8 \n\
        STORE rcx:8, 0x2:4 \n\
        CALL <func3> \n\
        RETURN \n\
        \n\
        \n\
        // func3(Object* param1) \n\
        <func3>: \n\
        $1:8 = INT_ADD rip:8, 0x20:8 \n\
        $2:4 = LOAD $1:8, 4:8 \n\
        $3:8 = INT_ADD rcx:8, 0x10:8 \n\
        STORE $3:8, $2:4 \n\
        $4:8 = INT_ADD rcx:8, 0x20:8 \n\
        STORE $4:8, 0x5:4 \n\
        RETURN \
    ";
    auto funcSig = "\
        funcSig = signature fastcall void(void* param1) \
    ";
    auto expectedIRCodeOfFunc1 = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[$U1]:8 = INT_ADD var1, 0x100:8 \n\
            var3:8 = LOAD var2 \n\
            var4[rcx]:8 = COPY var3 \n\
            var5[var4]:4 = COPY 0x1:4 \n\
            var6:1 = CALL 0xa00:8, var4 \
    ";
    auto expectedIRCodeOfFunc2 = "\
        Block B5(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[$U1]:8 = INT_ADD var1, 0x200:8 \n\
            var3:8 = LOAD var2 \n\
            var4[rcx]:8 = COPY var3 \n\
            var5[var4]:4 = COPY 0x2:4 \n\
            var6:1 = CALL 0xa00:8, var4 \
    ";
    auto expectedIRCodeOfFunc3 = "\
        Block Ba(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[$U1]:8 = INT_ADD var1, 0x20:8 \n\
            var3:4 = LOAD var2 \n\
            var4[$U2]:4 = COPY var3 \n\
            var5:8 = LOAD rcx // param1 \n\
            var6[$U3]:8 = INT_ADD var5, 0x10:8 \n\
            var7[var6]:4 = COPY var4 \n\
            var8[$U4]:8 = INT_ADD var5, 0x20:8 \n\
            var9[var8]:4 = COPY 0x5:4 \
    ";
    auto expectedDataFlowOfFunc1 = "\
        var1 <- Copy Start \n\
        var2 <- Copy var1 + 0x100 \n\
        var3 <- Read var2 \n\
        var4 <- Copy var3 \n\
        var5 <- Write var4 \n\
        var5 <- Write 0x1 \n\
        Ba:var5 <- Copy var4 \
    ";
    auto expectedDataFlowOfFunc2 = "\
        var1 <- Copy Start \n\
        var2 <- Copy var1 + 0x200 \n\
        var3 <- Read var2 \n\
        var4 <- Copy var3 \n\
        var5 <- Write var4 \n\
        var5 <- Write 0x2 \n\
        Ba:var5 <- Copy var4 \
    ";
    auto expectedDataFlowOfFunc3 = "\
        var1 <- Copy Start \n\
        var2 <- Copy var1 + 0x20 \n\
        var3 <- Read var2 \n\
        var4 <- Copy var3 \n\
        var5 <- Copy B0:var4 \n\
        var5 <- Copy B5:var4 \n\
        var6 <- Copy var5 + 0x10 \n\
        var7 <- Write var6 \n\
        var7 <- Write var4 \n\
        var8 <- Copy var5 + 0x20 \n\
        var9 <- Write var8 \n\
        var9 <- Write 0x5 \
    ";
    auto expectedStructuresBefore = "\
        struct root { \n\
            0x20: Ba:var3 \n\
            0x100: B0:var3 \n\
        } \n\
        \n\
        struct B0:var3 : root_0x100 { \n\
            0x0: 0x1 \n\
            0x10: Ba:var3 \n\
            0x20: 0x5 \n\
        } \
    ";
    auto expectedStructuresAfter = "\
        struct Ba:var5 { \n\
            0x10: Ba:var3 \n\
            0x20: 0x5 \n\
        } \n\
        \n\
        struct B0:var3 : Ba:var5, root_0x100 { \n\
            0x0: 0x1 \n\
        } \n\
        \n\
        struct B5:var3 : Ba:var5, root_0x200 { \n\
            0x0: 0x2 \n\
        } \n\
        \n\
        struct root { \n\
            0x20: Ba:var3 \n\
            0x100: B0:var3 \n\
            0x200: B5:var3 \n\
        } \
    ";
    auto instructions = PcodeFixture::parsePcode(sourcePCode);
    pcode::ListInstructionProvider provider(instructions);
    graph->explore(pcode::InstructionOffset(0, 0), &provider);
    auto func1 = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(0, 0)));
    auto func3 = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(10, 0)));
    auto funcSigDt = dynamic_cast<SignatureDataType*>(
        parseDataType(funcSig));
    func3->getFunctionSymbol()->getSignature()->copyFrom(funcSigDt);
    ASSERT_TRUE(cmp(func1, expectedIRCodeOfFunc1));
    ASSERT_TRUE(cmp(func3, expectedIRCodeOfFunc3));
    ASSERT_TRUE(cmpStructures(expectedStructuresBefore));

    graph->explore(pcode::InstructionOffset(5, 0), &provider);
    auto func2 = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(5, 0)));
    ASSERT_TRUE(cmp(func2, expectedIRCodeOfFunc2));
    ASSERT_TRUE(cmpDataFlow(func1, expectedDataFlowOfFunc1));
    ASSERT_TRUE(cmpDataFlow(func2, expectedDataFlowOfFunc2));
    ASSERT_TRUE(cmpDataFlow(func3, expectedDataFlowOfFunc3));
    ASSERT_TRUE(cmpStructures(expectedStructuresAfter));
}
