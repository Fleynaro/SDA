#pragma once
#include "SDA/Core/Pcode/PcodeParser.h"
#include "Test/Core/ContextFixture.h"

namespace sda::test
{
    class PcodeFixture : public ContextFixture
    {
    protected:
        std::list<pcode::Instruction> parsePcode(const std::string& text) const;

        void parsePcode(const std::string& text, pcode::Graph* graph) const;

        void printPcode(pcode::FunctionGraph* graph, std::ostream& out) const;
    };
};