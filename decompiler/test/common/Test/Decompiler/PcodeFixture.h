#pragma once
#include "SDA/Core/Pcode/PcodeParser.h"
#include "Test/Core/ContextFixture.h"

namespace sda::test
{
    class PcodeFixture : public ContextFixture
    {
    protected:
        std::list<pcode::Instruction> parsePcode(const std::string& text) const;
    };
};