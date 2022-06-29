#pragma once
#include "Core/IRcode/IRcodeBlock.h"
#include "Core/Pcode/PcodeInstruction.h"

namespace sda::decompiler
{
    class IRcodeBlockGenerator
    {
        struct MemorySpace {
            std::list<std::shared_ptr<ircode::Variable>> variables;
        };
    
        ircode::Block* m_block;
        std::map<ircode::Hash, MemorySpace> m_memorySpaces;
    public:
        IRcodeBlockGenerator(ircode::Block* block);

        void executePcode(pcode::Instruction* instr);

    private:
        void writeMemory(MemorySpace* memSpace, std::shared_ptr<ircode::Variable> variable);

        struct VariableReadInfo {
            std::shared_ptr<ircode::Variable> variable;
            size_t offset;
        };

        std::list<VariableReadInfo> readMemory(MemorySpace* memSpace, size_t readOffset, size_t readSize, BitMask& readMask);

        std::list<VariableReadInfo> readRegisterVarnode(const pcode::RegisterVarnode* regVarnode);

        std::shared_ptr<ircode::Value> readVarnode(const pcode::Varnode* varnode);

        MemorySpace* getMemSpace(ircode::Hash baseAddrHash);

        void generateOperation(std::unique_ptr<ircode::Operation> operation);
    };
};