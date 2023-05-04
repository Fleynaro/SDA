#include "Test/Core/IRcode/IRcodeFixture.h"
#include "Test/Core/Utils/TestAssertion.h"
#include "SDA/Core/Semantics/Semantics.h"
#include "SDA/Core/Utils/AbstractPrinter.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class SemanticsFixture : public IRcodeFixture
{
protected:
    semantics::SemanticsManager semManager = semantics::SemanticsManager(&program);
};