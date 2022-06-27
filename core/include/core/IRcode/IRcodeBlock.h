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
        std::list<Block*> m_previousBlocks;
        pcode::Block* m_pcodeBlock = nullptr;
    public:
        Block(pcode::Block* pcodeBlock);

        std::list<Operation>& getOperations();

        Block* getNearNextBlock() const;

        Block* getFarNextBlock() const;

        void setNextBlocks(Block* nearNextBlock, Block* farNextBlock);

        const std::list<Block*>& getPreviousBlocks() const;
    };
};