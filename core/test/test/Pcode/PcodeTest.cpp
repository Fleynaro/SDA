#include "Test/Core/Pcode/PcodeFixture.h"
#include "Test/Core/Utils/TestAssertion.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class PcodeTest : public PcodeFixture
{
protected:
    
};

TEST_F(PcodeTest, Sample1) {
    auto sourcePCode = "\
        rax:8 = COPY 0:8 \
        <label1>: \
        rax:8 = INT_ADD rax:8, 1:8 \
        BRANCH <label1> \
    ";
    pcode::Graph graph;
    parsePcode(sourcePCode, &graph);
    auto funcGraph = graph.getFunctionGraphAt(pcode::InstructionOffset(0));
    printPcode(funcGraph, std::cout);
}
