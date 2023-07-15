#include "Test/Core/Researchers/StructureResearcherFixture.h"
#include "SDA/Core/Researchers/ClassResearcher.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class ClassResearcherTest : public StructureResearcherFixture
{
protected:
    std::unique_ptr<researcher::ClassRepository> classRepo;
    std::unique_ptr<researcher::ClassResearcher> classResearcher;

    void SetUp() override {
        StructureResearcherFixture::SetUp();
        classRepo = std::make_unique<researcher::ClassRepository>(eventPipe);
        classResearcher = std::make_unique<researcher::ClassResearcher>(
            program,
            context->getPlatform(),
            classRepo.get(),
            structureRepo.get());
        eventPipe->connect(classResearcher->getEventPipe());
    }

    ::testing::AssertionResult cmpStructureInfos(const std::string& expectedCode) const {
        std::stringstream ss;
        for (auto structure : sortByName(structureRepo->getAllStructures())) {
            auto info = classRepo->getStructureInfo(structure);
            auto& values = info->conditions.values();
            if (values.empty()) continue;
            ss << structure->name << std::endl;
            for (auto& [offset, valuesAtOffset] : values) {
                std::string sep;
                std::stringstream fieldValues;
                for (auto value : valuesAtOffset) {
                    fieldValues << sep << "0x" << utils::ToHex(value);
                    sep = ", ";
                }
                ss << "    0x" << utils::ToHex(offset) << ": " << fieldValues.str() << std::endl;
            }
        }
        return Compare(ss.str(), expectedCode);
    }
};

TEST_F(ClassResearcherTest, Functions) {
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
    auto expectedStructureInfos = "\
        B0:var13 \n\
            0x0: 0x2 \n\
        B0:var8 \n\
            0x0: 0x1 \n\
        Bd:var1 \n\
            0x0: 0x1, 0x2 \n\
            0x4: 0x1 \n\
        Bd:var7 \n\
            0x0: 0x1 \n\
            0x4: 0x1 \n\
            0x10: 0x64 \
    ";
    auto mainFunction = parsePcode(sourcePCode, program);
    auto funcSigDt = dynamic_cast<SignatureDataType*>(
        parseDataType(funcSig));
    auto func = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(13, 0)));
    func->getFunctionSymbol()->getSignature()->copyFrom(funcSigDt);
    ASSERT_TRUE(cmp(mainFunction, expectedIRCodeOfMainFunc));
    ASSERT_TRUE(cmp(func, expectedIRCodeOfFunc));
    ASSERT_TRUE(cmpStructures(expectedStructures));
    ASSERT_TRUE(cmpStructureInfos(expectedStructureInfos));
}
