#pragma once
#include <list>
#include "IRcodeOperation.h"
#include "Core/Pcode/PcodeBlock.h"

namespace sda::ircode
{
    class Block
    {
        std::list<Operation> m_operations;
        Block* m_nearNextBlock = nullptr;
        Block* m_farNextBlock = nullptr;
        std::list<Block*> m_referencedBlocks;
        pcode::Block* m_pcodeBlock = nullptr;
    public:
        Block(pcode::Block* pcodeBlock);

        std::list<Operation>& getOperations();

        Block* getNearNextBlock() const;

        Block* getFarNextBlock() const;

        void setNearNextBlocks(Block* nearNextBlock, Block* farNextBlock);
    };
};