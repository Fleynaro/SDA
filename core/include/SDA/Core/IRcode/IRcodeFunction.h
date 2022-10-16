#pragma once
#include <list>
#include "IRcodeBlock.h"
#include "SDA/Core/Pcode/PcodeFunctionGraph.h"

namespace sda::ircode
{
    class Function
    {
        std::list<Block> m_blocks;
        pcode::FunctionGraph* m_functionGraph = nullptr;
    public:
        Function(pcode::FunctionGraph* functionGraph);

        std::list<Block>& getBlocks();
    };
};