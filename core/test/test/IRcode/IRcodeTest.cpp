#include "Test/Core/IRcode/IRcodeFixture.h"
#include "Test/Core/Utils/TestAssertion.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class IRcodeTest : public IRcodeFixture
{
protected:
    ::testing::AssertionResult cmp(ircode::Function* function, const std::string& expectedCode) const {
        std::stringstream ss;
        printIRcode(function, ss, 2);
        return Compare(ss.str(), expectedCode);
    }
};

TEST_F(IRcodeTest, Simple) {
    // Graph: https://photos.app.goo.gl/j2VRr1jGHpA8EaaY9
    auto sourcePCode = "\
        NOP // block 0 \n\
        <label>: \n\
        NOP // block 1 \n\
        BRANCH <label> \
    ";
    auto expectedPCode = "\
        Block B0(level: 1, near: B1): \n\
            NOP \n\
        Block B1(level: 2, far: B1): \n\
            NOP \n\
            BRANCH <B1>:8 \
    ";
    pcode::Graph graph;
    ircode::Program program(&graph);
    auto funcGraph = parsePcode(sourcePCode, &graph);
    
}