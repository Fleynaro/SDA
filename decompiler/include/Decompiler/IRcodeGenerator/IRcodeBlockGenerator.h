#pragma once
#include "Core/IRcode/IRcodeBlock.h"
#include "Core/Pcode/PcodeInstruction.h"

namespace sda::decompiler
{
    struct MemorySpace {
        std::list<std::shared_ptr<ircode::Variable>> variables;
    };

    class MemorySpacePool
    {
        std::map<ircode::Hash, MemorySpace> m_memorySpaces;
    public:
        MemorySpace* getMemSpace(ircode::Hash baseAddrHash);
    };

    class IRcodeBlockGenerator
    {
        ircode::Block* m_block;
        MemorySpacePool* m_memSpacePool;
    public:
        IRcodeBlockGenerator(ircode::Block* block, MemorySpacePool* memSpacePool);

        void executePcode(pcode::Instruction* instr);

    private:
        void genWriteMemory(MemorySpace* memSpace, std::shared_ptr<ircode::Variable> variable);

        struct VariableReadInfo {
            std::shared_ptr<ircode::Variable> variable;
            size_t offset;
        };

        std::list<VariableReadInfo> genReadMemory(MemorySpace* memSpace, size_t readOffset, size_t readSize, BitMask& readMask);

        std::list<VariableReadInfo> genReadRegisterVarnode(const pcode::RegisterVarnode* regVarnode);

        std::shared_ptr<ircode::Value> genReadVarnode(const pcode::Varnode* varnode);

        void genOperation(std::unique_ptr<ircode::Operation> operation);
    };
};