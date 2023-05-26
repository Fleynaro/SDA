#pragma once
#include <list>
#include "IRcodeOperation.h"
#include "IRcodeMemory.h"
#include "SDA/Core/Pcode/PcodeBlock.h"

namespace sda::ircode
{
    class Function;

    class Block
    {
        pcode::Block* m_pcodeBlock = nullptr;
        Function* m_function = nullptr;
        std::list<std::unique_ptr<Operation>> m_operations;
        MemorySpace m_memSpace;
        utils::BitSet m_varIds;
        Hash m_hash = 0;
        Hash m_dominantHash = 0;
        std::shared_ptr<Value> m_condition;
    public:
        Block(pcode::Block* pcodeBlock, Function* function);

        Function* getFunction() const;

        Hash getHash() const;

        pcode::Block* getPcodeBlock() const;

        std::string getName() const;

        size_t getIndex() const;

        std::list<std::unique_ptr<Operation>>& getOperations();

        MemorySpace* getMemorySpace();

        Block* getNearNextBlock() const;

        Block* getFarNextBlock() const;

        std::list<Block*> getReferencedBlocks() const;

        std::list<Block*> getDominantBlocks() const;

        std::shared_ptr<Value>& getCondition();

        void clear();

        void passDescendants(std::function<void(Block* block, bool& goNextBlocks)> callback);

        void update();

    private:
        void decompile(bool& goNextBlocks);

        Hash calcHash();

        Hash calcDominantHash();

        size_t getNextVarId();

        void clearVarIds();

        void replaceWith(Block* block);
    };
};