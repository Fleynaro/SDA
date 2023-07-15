#include "ResearcherFixture.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

void SemanticsFixture::SetUp() {
    IRcodeFixture::SetUp();
    semManager = new semantics::SemanticsManager(program);
}

void SemanticsFixture::TearDown() {
    delete semManager;
    IRcodeFixture::TearDown();
}
