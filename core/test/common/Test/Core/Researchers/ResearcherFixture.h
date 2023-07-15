#pragma once
#include "Test/Core/IRcode/IRcodeFixture.h"
#include "Test/Core/Utils/TestAssertion.h"
#include "SDA/Core/Researchers/ResearcherHelper.h"
#include "SDA/Core/Utils/AbstractPrinter.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class ResearcherFixture : public IRcodeFixture
{
protected:
    void SetUp() override;

    void TearDown() override;
};