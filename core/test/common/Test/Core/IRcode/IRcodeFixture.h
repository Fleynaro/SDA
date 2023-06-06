#pragma once
#include "SDA/Core/IRcode/IRcodeProgram.h"
#include "SDA/Core/IRcode/IRcodeContextSync.h"
#include "Test/Core/Pcode/PcodeFixture.h"

namespace sda::test
{
    class IRcodeFixture : public PcodeFixture
    {
    protected:
        ircode::Program* program = nullptr;

        void SetUp() override;

        void TearDown() override;
        
        void printIRcode(ircode::Function* function, std::ostream& out, size_t tabs = 0) const;

        ircode::Function* parsePcode(const std::string& text, ircode::Program* program) const;

        ::testing::AssertionResult cmp(ircode::Function* function, const std::string& expectedCode) const;
    };
};