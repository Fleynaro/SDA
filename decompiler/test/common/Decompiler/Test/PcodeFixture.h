#pragma once
#include "Core/Pcode/PcodeParser.h"
#include "Core/Test/ContextFixture.h"

namespace sda::test
{
    class PcodeFixture : public ContextFixture
    {
    protected:
        std::list<pcode::Instruction> parsePcode(const std::string& text) const;
    };
};