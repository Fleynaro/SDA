#pragma once
#include "SDA/Core/IRcode/IRcodeProgram.h"
#include "SDA/Core/IRcode/IRcodeContextSync.h"
#include "SDA/Core/IRcode/IRcodePcodeSync.h"
#include "Test/Core/Pcode/PcodeFixture.h"

namespace sda::test
{
    class IRcodeFixture : public PcodeFixture
    {
    protected:
        ircode::Program* program = nullptr;
        std::unique_ptr<ircode::ContextSync> ctxSync;
        std::unique_ptr<ircode::PcodeSync> pcodeSync;
        bool printVarAddressAlways = false;

        void SetUp() override;

        void TearDown() override;
        
        ircode::Function* parsePcode(const std::string& text, ircode::Program* program) const;

        ::testing::AssertionResult cmp(ircode::Function* function, const std::string& expectedCode) const;
    };
};