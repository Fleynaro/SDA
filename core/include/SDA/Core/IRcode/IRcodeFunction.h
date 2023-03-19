#pragma once
#include <list>
#include "IRcodeBlock.h"
#include "SDA/Core/Pcode/PcodeFunctionGraph.h"

namespace sda::ircode
{
    class Function
    {
        friend class Block;
        pcode::FunctionGraph* m_functionGraph = nullptr;
        std::map<pcode::Block*, Block> m_blocks;
        utils::BitSet m_varIds;
    public:
        Function(pcode::FunctionGraph* functionGraph);

        pcode::FunctionGraph* getFunctionGraph() const;

        std::map<pcode::Block*, Block>& getBlocks();

        Block* toBlock(pcode::Block* pcodeBlock);
    };
};