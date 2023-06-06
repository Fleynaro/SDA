#pragma once
#include "SDA/Core/Pcode/PcodeParser.h"
#include "Test/Core/ContextFixture.h"

namespace sda::test
{
    class PcodeFixture : public ContextFixture
    {
    protected:
        pcode::Graph* graph = nullptr;

        void SetUp() override;

        void TearDown() override;

        std::list<pcode::Instruction> parsePcode(const std::string& text) const;

        pcode::FunctionGraph* parsePcode(const std::string& text, pcode::Graph* graph) const;

        void printPcode(pcode::FunctionGraph* graph, std::ostream& out, size_t tabs = 0) const;

        ::testing::AssertionResult cmp(pcode::FunctionGraph* funcGraph, const std::string& expectedCode) const;
    };
};