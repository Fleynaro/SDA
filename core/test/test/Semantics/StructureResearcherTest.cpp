#include "Test/Core/Semantics/SemanticsFixture.h"
#include "SDA/Core/Semantics/StructureResearcher.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class StructureResearcherTest : public SemanticsFixture
{
protected:
    std::unique_ptr<semantics::StructureRepository> structureRepo;
    std::unique_ptr<semantics::StructureResearcher> structureResearcher;

    void SetUp() override {
        SemanticsFixture::SetUp();
        structureRepo = std::make_unique<semantics::StructureRepository>();
        structureResearcher = std::make_unique<semantics::StructureResearcher>(
            program,
            context->getPlatform(),
            structureRepo.get());
    }
};

TEST_F(StructureResearcherTest, Simple1) {
    auto sourcePCode = "\
        // main() \n\
        xmm0:Da = COPY 0.5:4 \n\
        CALL <setGlobalFloatValue> \n\
        RETURN \n\
        \n\
        \n\
        // setGlobalFloatValue(float value) \n\
        <setGlobalFloatValue>: \n\
        $0:8 = INT_ADD rip:8, 0x10:8 \n\
        STORE $0:8, xmm0:Da \n\
        RETURN \n\
    ";
}
