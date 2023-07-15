#pragma once
#include "Test/Core/IRcode/IRcodeFixture.h"
#include "Test/Core/Utils/TestAssertion.h"
#include "SDA/Core/Researchers/Researcher.h"
#include "SDA/Core/Utils/AbstractPrinter.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class SemanticsFixture : public IRcodeFixture
{
protected:
    semantics::SemanticsManager* semManager = nullptr;

    void SetUp() override;

    void TearDown() override;
};