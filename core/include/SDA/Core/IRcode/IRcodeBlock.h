#pragma once
#include <list>
#include "IRcodeOperation.h"
#include "SDA/Core/Pcode/PcodeBlock.h"

namespace sda::ircode
{
    class Block
    {
        std::list<std::unique_ptr<Operation>> m_operations;
        Block* m_nearNextBlock = nullptr;
        Block* m_farNextBlock = nullptr;
        std::list<Block*> m_previousBlocks;
        pcode::Block* m_pcodeBlock = nullptr;
    public:
        Block(pcode::Block* pcodeBlock);

        std::list<std::unique_ptr<Operation>>& getOperations();

        Block* getNearNextBlock() const;

        Block* getFarNextBlock() const;

        void setNextBlocks(Block* nearNextBlock, Block* farNextBlock);

        const std::list<Block*>& getPreviousBlocks() const;
    };
};