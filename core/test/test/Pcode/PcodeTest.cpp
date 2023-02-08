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
    auto sourcePCode = "\
        NOP \n\
        <label1>: \n\
        NOP \n\
        BRANCH <label1> \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B1): \n\
            NOP \n\
        Block B1(level: 2, far: B1): \n\
            NOP \n\
            BRANCH <B1>:8 \
    ";
    pcode::Graph graph;
    parsePcode(sourcePCode, &graph);
    auto funcGraph = graph.getFunctionGraphAt(pcode::InstructionOffset(0));
    ASSERT_TRUE(cmp(funcGraph, expectedPCode));
}
