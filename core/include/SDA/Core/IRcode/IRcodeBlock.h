#pragma once
#include <list>
#include "IRcodeOperation.h"
#include "SDA/Core/Pcode/PcodeBlock.h"

namespace sda::ircode
{
    class Function;

    class Block
    {
        pcode::Block* m_pcodeBlock = nullptr;
        Function* m_function = nullptr;
        std::list<std::unique_ptr<Operation>> m_operations;
    public:
        Block(pcode::Block* pcodeBlock, Function* function);

        pcode::Block* getPcodeBlock() const;

        std::string getName() const;

        std::list<std::unique_ptr<Operation>>& getOperations();

        Block* getNearNextBlock() const;

        Block* getFarNextBlock() const;

        std::list<Block*> getReferencedBlocks() const;

        void update();
    };
};