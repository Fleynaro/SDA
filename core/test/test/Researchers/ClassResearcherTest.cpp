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
            auto values = info->getLabelSet().values();
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

    ::testing::AssertionResult cmpFieldStructureGroups(const std::string& expectedCode) const {
        std::list<std::string> strings;
        for (auto& group : classRepo->getAllFieldStructureGroups()) {
            if (group.structures.size() == 1) continue;
            std::string sep;
            std::stringstream ss;
            for (auto structure : sortByName(group.structures)) {
                ss << sep << structure->name;
                sep = ", ";
            }
            strings.push_back(ss.str());
        }
        strings.sort();
        std::stringstream ss;
        for (auto& str : strings) {
            ss << "{ " << str << " }" << std::endl;
        }
        return Compare(ss.str(), expectedCode);
    }
};

::testing::AssertionResult CmpClassFieldRangeSet(const researcher::ClassFieldRangeSet::Ranges& ranges, const std::string& expectedCode) {
    std::stringstream ss;
    for (auto& [labels, maxOffset] : ranges) {
        ss << "0x0 - 0x" << utils::ToHex(maxOffset);
        std::string sep;
        std::stringstream labelsStr;
        for (auto label : labels) {
            labelsStr << sep << "0x" << utils::ToHex(label);
            sep = ", ";
        }
        ss << ": " << labelsStr.str() << std::endl;
    }
    return Compare(ss.str(), expectedCode);
}

TEST(ClassFieldRangeSet, test1) {
    researcher::ClassFieldRangeSet set;
    set.addRange({ 1 }, 0x1000);
    set.addRange({ 2 }, 0x1000);
    set.addRange({ 3 }, 0x1000);
    set.addRange({ 4 }, 0x1000);
    set.addRange({ 1, 2 }, 0x10);
    set.addRange({ 1, 2 }, 0x18); // overrides previous
    set.addRange({ 1, 2 }, 0x4); // no effect
    set.addRange({ 2, 3 }, 0x8); // order 1
    set.addRange({ 3, 4 }, 0x20); // order 1
    set.finalize();
    auto expectedAllRanges = "\
        0x0 - 0x8: 0x1, 0x2, 0x3, 0x4 \n\
        0x0 - 0x20: 0x3, 0x4 \n\
        0x0 - 0x18: 0x1, 0x2 \n\
        0x0 - 0x1000: 0x4 \n\
        0x0 - 0x1000: 0x3 \n\
        0x0 - 0x1000: 0x2 \n\
        0x0 - 0x1000: 0x1 \
    ";
    ASSERT_TRUE(CmpClassFieldRangeSet(set.getAllRanges(), expectedAllRanges));
    auto expectedRangesFor0x1 = "\
        0x0 - 0x8: 0x1, 0x2, 0x3, 0x4 \
    ";
    ASSERT_TRUE(CmpClassFieldRangeSet(set.getRangesContaining(0x1), expectedRangesFor0x1));
    auto expectedRangesFor0x14 = "\
        0x0 - 0x20: 0x3, 0x4 \n\
        0x0 - 0x18: 0x1, 0x2 \
    ";
    ASSERT_TRUE(CmpClassFieldRangeSet(set.getRangesContaining(0x14), expectedRangesFor0x14));
    auto expectedRangesFor0x500 = "\
        0x0 - 0x1000: 0x4 \n\
        0x0 - 0x1000: 0x3 \n\
        0x0 - 0x1000: 0x2 \n\
        0x0 - 0x1000: 0x1 \
    ";
    ASSERT_TRUE(CmpClassFieldRangeSet(set.getRangesContaining(0x500), expectedRangesFor0x500));
}

TEST(ClassFieldRangeSet, test2) {
    // the same as test1 but with different order
    researcher::ClassFieldRangeSet set;
    set.addRange({ 1 }, 0x1000);
    set.addRange({ 2 }, 0x1000);
    set.addRange({ 3 }, 0x1000);
    set.addRange({ 4 }, 0x1000);
    set.addRange({ 1, 2 }, 0x18);
    set.addRange({ 3, 4 }, 0x20); // order 2
    set.addRange({ 2, 3 }, 0x8); // order 2
    set.finalize();
    auto expectedAllRanges = "\
        0x0 - 0x8: 0x1, 0x2, 0x3, 0x4 \n\
        0x0 - 0x20: 0x3, 0x4 \n\
        0x0 - 0x18: 0x1, 0x2 \n\
        0x0 - 0x1000: 0x4 \n\
        0x0 - 0x1000: 0x3 \n\
        0x0 - 0x1000: 0x2 \n\
        0x0 - 0x1000: 0x1 \
    ";
    ASSERT_TRUE(CmpClassFieldRangeSet(set.getAllRanges(), expectedAllRanges));
}

TEST(ClassFieldRangeSet, test3) {
    researcher::ClassFieldRangeSet set;
    set.addRange({ 1 }, 0x1000);
    set.addRange({ 2 }, 0x1000);
    set.addRange({ 3 }, 0x1000);
    set.addRange({ 4 }, 0x1000);
    set.addRange({ 1, 2 }, 0x18);
    set.addRange({ 3, 4 }, 0x20);
    set.addRange({ 2, 3 }, 0x8);
    set.addRange({ 1, 2, 3 }, 0x20);
    set.finalize();
    auto expectedAllRanges = "\
        0x0 - 0x20: 0x1, 0x2, 0x3, 0x4 \n\
        0x0 - 0x1000: 0x4 \n\
        0x0 - 0x1000: 0x3 \n\
        0x0 - 0x1000: 0x2 \n\
        0x0 - 0x1000: 0x1 \
    ";
    ASSERT_TRUE(CmpClassFieldRangeSet(set.getAllRanges(), expectedAllRanges));
}

TEST_F(ClassResearcherTest, Test1) {
    /*
        void main() {
            create1();
            create2();
            globalVar_0x100->field_0x8 = globalVar_0x20;
            if (globalVar_0x100->field_0x0 == 1) {
                player = static_cast<Player*>(globalVar_0x100);
                player->field_0x8 = globalVar_0x30;
                player->field_0x10 = 5;
            }
        }

        void create1() {
            globalVar_0x1000->field_0x0 = 1;
            globalVar_0x100 = globalVar_0x1000;
        }

        void create2() {
            globalVar_0x2000->field_0x0 = 2;
            globalVar_0x100 = globalVar_0x2000;
        }
    */
   auto sourcePCode = "\
        // main() \n\
        CALL <create1> \n\
        CALL <create2> \n\
        $1:8 = INT_ADD rip:8, 0x100:8 \n\
        $2:8 = LOAD $1:8, 8:8 \n\
        $3:8 = INT_ADD rip:8, 0x20:8 \n\
        $4:8 = LOAD $3:8, 8:8 \n\
        $5:8 = INT_ADD $2:8, 0x8:8 \n\
        STORE $5:8, $4:8 \n\
        $6:4 = LOAD $2:8, 4:8 \n\
        $7:1 = INT_NOTEQUAL $6:4, 1:4 \n\
        CBRANCH <end>, $7:1 \n\
        $8:8 = INT_ADD $2:8, 0x8:8 \n\
        $9:8 = INT_ADD rip:8, 0x30:8 \n\
        $10:4 = LOAD $9:8, 4:8 \n\
        STORE $8:8, $10:4 \n\
        $11:8 = INT_ADD $2:8, 0x10:8 \n\
        STORE $11:8, 5:4 \n\
        <end>: \n\
        RETURN \n\
        \n\
        \n\
        // create1() \n\
        <create1>: \n\
        $1:8 = INT_ADD rip:8, 0x1000:8 \n\
        $2:8 = LOAD $1:8, 8:8 \n\
        STORE $2:8, 1:4 \n\
        $3:8 = INT_ADD rip:8, 0x100:8 \n\
        STORE $3:8, $2:8 \n\
        RETURN \n\
        \n\
        \n\
        // create2() \n\
        <create2>: \n\
        $1:8 = INT_ADD rip:8, 0x2000:8 \n\
        $2:8 = LOAD $1:8, 8:8 \n\
        STORE $2:8, 2:4 \n\
        $3:8 = INT_ADD rip:8, 0x100:8 \n\
        STORE $3:8, $2:8 \n\
        RETURN \
    ";
    auto expectedIRCodeOfMainFunc = "\
        Block B0(level: 1, near: Bb, far: B11, cond: var14): \n\
            var1:1 = CALL 0x1200:8 \n\
            var2:1 = CALL 0x1800:8 \n\
            var3:8 = LOAD rip \n\
            var4[$U1]:8 = INT_ADD var3, 0x100:8 \n\
            var5:8 = LOAD var4 \n\
            var6[$U2]:8 = COPY var5 \n\
            var7[$U3]:8 = INT_ADD var3, 0x20:8 \n\
            var8:8 = LOAD var7 \n\
            var9[$U4]:8 = COPY var8 \n\
            var10[$U5]:8 = INT_ADD var6, 0x8:8 \n\
            var11[var10]:8 = COPY var9 \n\
            var12:4 = LOAD var6 \n\
            var13[$U6]:4 = COPY var12 \n\
            var14[$U7]:1 = INT_NOTEQUAL var13, 0x1:4 \n\
        Block Bb(level: 2, near: B11): \n\
            var15:8 = REF var6 \n\
            var16[$U8]:8 = INT_ADD var15, 0x8:8 \n\
            var17:8 = REF var3 \n\
            var18[$U9]:8 = INT_ADD var17, 0x30:8 \n\
            var19:4 = LOAD var18 \n\
            var20[$U10]:4 = COPY var19 \n\
            var21[var16]:4 = COPY var20 \n\
            var22[$U11]:8 = INT_ADD var15, 0x10:8 \n\
            var23[var22]:4 = COPY 0x5:4 \n\
        Block B11(level: 3): \n\
            empty \
    ";
    auto expectedIRCodeOfCreate1 = "\
        Block B12(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[$U1]:8 = INT_ADD var1, 0x1000:8 \n\
            var3:8 = LOAD var2 \n\
            var4[$U2]:8 = COPY var3 \n\
            var5[var4]:4 = COPY 0x1:4 \n\
            var6[$U3]:8 = INT_ADD var1, 0x100:8 \n\
            var7[var6]:8 = COPY var4 \
    ";
    auto expectedIRCodeOfCreate2 = "\
        Block B18(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[$U1]:8 = INT_ADD var1, 0x2000:8 \n\
            var3:8 = LOAD var2 \n\
            var4[$U2]:8 = COPY var3 \n\
            var5[var4]:4 = COPY 0x2:4 \n\
            var6[$U3]:8 = INT_ADD var1, 0x100:8 \n\
            var7[var6]:8 = COPY var4 \
    ";
    auto expectedStructures = "\
        struct root { \n\
            0x20: B0:var8 \n\
            0x30: B0:var19 \n\
            0x100: B0:var5, B12:var3, B18:var3 \n\
            0x1000: B12:var3 \n\
            0x2000: B18:var3 \n\
        } \n\
        \n\
        struct B0:var5 : root_0x100 { \n\
            0x0: B0:var12 \n\
            0x8: B0:var8 \n\
        } \n\
        \n\
        struct B12:var3 : root_0x100, root_0x1000 { \n\
            0x0: 0x1 \n\
        } \n\
        \n\
        struct B18:var3 : root_0x100, root_0x2000 { \n\
            0x0: 0x2 \n\
        } \n\
        \n\
        struct B0:var15 : B0:var5 { \n\
            0x0: 0x1 \n\
            0x8: B0:var19 \n\
            0x10: 0x5 \n\
        } \
    ";
    auto expectedStructureInfos = "\
        B0:var15 \n\
            0x0: 0x1 \n\
            0x10: 0x5 \n\
        B0:var5 \n\
            0x0: 0x1, 0x2 \n\
        B12:var3 \n\
            0x0: 0x1 \n\
        B18:var3 \n\
            0x0: 0x2 \n\
        root_0x100 \n\
            0x0: 0x1, 0x2 \
    ";
    auto expectedFieldStructureGroups = "\
        { B0:var15_0x8, B0:var5_0x8 } \
    ";
    auto mainFunction = parsePcode(sourcePCode, program);
    auto create1 = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(18, 0)));
    auto create2 = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(24, 0)));
    ASSERT_TRUE(cmp(mainFunction, expectedIRCodeOfMainFunc));
    ASSERT_TRUE(cmp(create1, expectedIRCodeOfCreate1));
    ASSERT_TRUE(cmp(create2, expectedIRCodeOfCreate2));
    ASSERT_TRUE(cmpStructures(expectedStructures));
    ASSERT_TRUE(cmpStructureInfos(expectedStructureInfos));
    ASSERT_TRUE(cmpFieldStructureGroups(expectedFieldStructureGroups));
}

TEST_F(ClassResearcherTest, MutualFieldInTwoClasses) {
    /*
        void main() {
            globalVar_0x100->field_0x8 = globalVar_0x20;
            if (globalVar_0x100->field_0x0 == 1) {
                player = static_cast<Player*>(globalVar_0x100);
                player->field_0x8 = globalVar_0x30;
                player->field_0x10 = 5;
            }
            else if (globalVar_0x100->field_0x0 == 2) {
                vehicle = static_cast<Vehicle*>(globalVar_0x100);
                vehicle->field_0x8 = globalVar_0x40;
                vehicle->field_0x14 = 6;
            }
        }
    */
   auto sourcePCode = "\
        // main() \n\
        $1:8 = INT_ADD rip:8, 0x100:8 \n\
        $2:8 = LOAD $1:8, 8:8 \n\
        $3:8 = INT_ADD rip:8, 0x20:8 \n\
        $4:8 = LOAD $3:8, 8:8 \n\
        $5:8 = INT_ADD $2:8, 0x8:8 \n\
        STORE $5:8, $4:8 \n\
        $6:4 = LOAD $2:8, 4:8 \n\
        $7:1 = INT_NOTEQUAL $6:4, 1:4 \n\
        CBRANCH <vehicle_check>, $7:1 \n\
        $8:8 = INT_ADD $2:8, 0x8:8 \n\
        $9:8 = INT_ADD rip:8, 0x30:8 \n\
        $10:4 = LOAD $9:8, 4:8 \n\
        STORE $8:8, $10:4 \n\
        $11:8 = INT_ADD $2:8, 0x10:8 \n\
        STORE $11:8, 5:4 \n\
        BRANCH <end> \n\
        <vehicle_check>: \n\
        $12:1 = INT_NOTEQUAL $6:4, 2:4 \n\
        CBRANCH <end>, $12:1 \n\
        $13:8 = INT_ADD $2:8, 0x8:8 \n\
        $14:8 = INT_ADD rip:8, 0x40:8 \n\
        $15:4 = LOAD $14:8, 4:8 \n\
        STORE $13:8, $15:4 \n\
        $16:8 = INT_ADD $2:8, 0x14:8 \n\
        STORE $16:8, 6:4 \n\
        <end>: \n\
        RETURN \n\
    ";
    auto expectedIRCodeOfMainFunc = "\
        Block B0(level: 1, near: B9, far: B10, cond: var12): \n\
            var1:8 = LOAD rip \n\
            var2[$U1]:8 = INT_ADD var1, 0x100:8 \n\
            var3:8 = LOAD var2 \n\
            var4[$U2]:8 = COPY var3 \n\
            var5[$U3]:8 = INT_ADD var1, 0x20:8 \n\
            var6:8 = LOAD var5 \n\
            var7[$U4]:8 = COPY var6 \n\
            var8[$U5]:8 = INT_ADD var4, 0x8:8 \n\
            var9[var8]:8 = COPY var7 \n\
            var10:4 = LOAD var4 \n\
            var11[$U6]:4 = COPY var10 \n\
            var12[$U7]:1 = INT_NOTEQUAL var11, 0x1:4 \n\
        Block B9(level: 2, far: B18): \n\
            var13:8 = REF var4 \n\
            var14[$U8]:8 = INT_ADD var13, 0x8:8 \n\
            var15:8 = REF var1 \n\
            var16[$U9]:8 = INT_ADD var15, 0x30:8 \n\
            var17:4 = LOAD var16 \n\
            var18[$U10]:4 = COPY var17 \n\
            var19[var14]:4 = COPY var18 \n\
            var20[$U11]:8 = INT_ADD var13, 0x10:8 \n\
            var21[var20]:4 = COPY 0x5:4 \n\
        Block B10(level: 2, near: B12, far: B18, cond: var23): \n\
            var22:4 = REF var11 \n\
            var23[$U12]:1 = INT_NOTEQUAL var22, 0x2:4 \n\
        Block B12(level: 3, near: B18): \n\
            var24:8 = REF var4 \n\
            var25[$U13]:8 = INT_ADD var24, 0x8:8 \n\
            var26:8 = REF var1 \n\
            var27[$U14]:8 = INT_ADD var26, 0x40:8 \n\
            var28:4 = LOAD var27 \n\
            var29[$U15]:4 = COPY var28 \n\
            var30[var25]:4 = COPY var29 \n\
            var31[$U16]:8 = INT_ADD var24, 0x14:8 \n\
            var32[var31]:4 = COPY 0x6:4 \n\
        Block B18(level: 4): \n\
            empty \
    ";
    auto expectedStructures = "\
        struct root { \n\
            0x20: B0:var6 \n\
            0x30: B0:var17 \n\
            0x40: B0:var28 \n\
            0x100: B0:var3 \n\
        } \n\
        \n\
        struct B0:var3 : root_0x100 { \n\
            0x0: B0:var10 \n\
            0x8: B0:var6 \n\
        } \n\
        \n\
        struct B0:var13 : B0:var3 { \n\
            0x0: 0x1 \n\
            0x8: B0:var17 \n\
            0x10: 0x5 \n\
        } \n\
        \n\
        struct B0:var24 : B0:var3 { \n\
            0x0: 0x2 \n\
            0x8: B0:var28 \n\
            0x14: 0x6 \n\
        } \
    ";
    auto expectedStructureInfos = "\
        B0:var13 \n\
            0x0: 0x1 \n\
            0x10: 0x5 \n\
        B0:var24 \n\
            0x0: 0x2 \n\
            0x14: 0x6 \n\
        B0:var3 \n\
            0x0: 0x1, 0x2 \n\
        root_0x100 \n\
            0x0: 0x1, 0x2 \
    ";
    auto expectedFieldStructureGroups = "\
        { B0:var13_0x8, B0:var24_0x8, B0:var3_0x8 } \
    ";
    auto mainFunction = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(mainFunction, expectedIRCodeOfMainFunc));
    ASSERT_TRUE(cmpStructures(expectedStructures));
    ASSERT_TRUE(cmpStructureInfos(expectedStructureInfos));
    ASSERT_TRUE(cmpFieldStructureGroups(expectedFieldStructureGroups));
}
