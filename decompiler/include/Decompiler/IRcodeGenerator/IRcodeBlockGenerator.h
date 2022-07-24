#pragma once
#include "Core/IRcode/IRcodeBlock.h"
#include "Core/Pcode/PcodeInstruction.h"

namespace sda::decompiler
{
    struct MemorySpace {
        std::list<std::shared_ptr<ircode::Variable>> variables;
    };

    class TotalMemorySpace
    {
        std::map<ircode::Hash, MemorySpace> m_memorySpaces;
    public:
        MemorySpace* getMemSpace(ircode::Hash baseAddrHash);
    };

    class IRcodeBlockGenerator
    {
        ircode::Block* m_block;
        TotalMemorySpace* m_totalMemSpace;
    public:
        IRcodeBlockGenerator(ircode::Block* block, TotalMemorySpace* totalMemSpace);

        void executePcode(pcode::Instruction* instr);

    private:
        void genWriteMemory(MemorySpace* memSpace, std::shared_ptr<ircode::Variable> variable);

        struct VariableReadInfo {
            std::shared_ptr<ircode::Variable> variable;
            size_t offset;
        };

        std::list<VariableReadInfo> genReadMemory(MemorySpace* memSpace, size_t readOffset, size_t readSize, BitMask& readMask);

        ircode::MemoryAddress getRegisterMemoryAddress(const pcode::RegisterVarnode* regVarnode) const;

        std::list<VariableReadInfo> genReadRegisterVarnode(const pcode::RegisterVarnode* regVarnode);

        std::shared_ptr<ircode::Value> genReadVarnode(const pcode::Varnode* varnode);

        void genOperation(std::unique_ptr<ircode::Operation> operation);

        std::shared_ptr<ircode::Constant> createConstant(const pcode::ConstantVarnode* constVarnode) const;

        std::shared_ptr<ircode::Register> createRegister(const pcode::RegisterVarnode* regVarnode, const ircode::MemoryAddress& memAddr) const;

        std::shared_ptr<ircode::Variable> createVariable(const ircode::MemoryAddress& memAddress, ircode::Hash hash, size_t size) const;
    };
};