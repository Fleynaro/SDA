#pragma once
#include <list>
#include "IRcodeBlock.h"

namespace sda::ircode
{
    class Function
    {
        std::list<Block> m_blocks;
        pcode::FunctionGraph* m_pcodeFunctionGraph = nullptr;
    public:
        Function(pcode::FunctionGraph* pcodeFunctionGraph);

        std::list<Block>& getBlocks() const;
    };
};