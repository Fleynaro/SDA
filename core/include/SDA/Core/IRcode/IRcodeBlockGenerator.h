#pragma once
#include "SDA/Core/IRcode/IRcodeBlock.h"
#include "SDA/Core/IRcode/IRcodeDataTypeProvider.h"
#include "SDA/Core/Pcode/PcodeInstruction.h"

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
        ircode::DataTypeProvider* m_dataTypeProvider;
        size_t m_nextVarId;
        const pcode::Instruction* m_curInstr = nullptr;
        std::set<std::shared_ptr<ircode::Variable>> m_overwrittenVariables;
        std::list<ircode::Operation*> m_genOperations;
    public:
        IRcodeBlockGenerator(
            ircode::Block* block,
            TotalMemorySpace* totalMemSpace,
            ircode::DataTypeProvider* dataTypeProvider,
            size_t nextVarId = 1);

        void executePcode(const pcode::Instruction* instr);

        const std::list<ircode::Operation*>& getGeneratedOperations() const;

    private:
        void genWriteMemory(MemorySpace* memSpace, std::shared_ptr<ircode::Variable> variable);

        struct VariableReadInfo {
            std::shared_ptr<ircode::Variable> variable;
            Offset offset;
        };

        std::list<VariableReadInfo> genReadMemory(MemorySpace* memSpace, Offset readOffset, size_t readSize, utils::BitMask& readMask);

        ircode::MemoryAddress getRegisterMemoryAddress(const Register& reg) const;

        ircode::MemoryAddress getMemoryAddress(std::shared_ptr<ircode::Value> addrValue) const;

        std::list<VariableReadInfo> genReadRegisterVarnode(std::shared_ptr<pcode::RegisterVarnode> regVarnode);

        std::shared_ptr<ircode::Value> genReadVarnode(std::shared_ptr<pcode::Varnode> varnode);

        void genOperation(std::unique_ptr<ircode::Operation> operation);

        void genGenericOperation(const pcode::Instruction* instr, ircode::OperationId operationId, ircode::MemoryAddress& outputMemAddr);

        std::shared_ptr<ircode::Variable> genLoadOperation(const ircode::MemoryAddress& memAddr, size_t loadSize);

        std::shared_ptr<ircode::Constant> createConstant(std::shared_ptr<pcode::ConstantVarnode> constVarnode) const;

        std::shared_ptr<ircode::Register> createRegister(std::shared_ptr<pcode::RegisterVarnode> regVarnode, const ircode::MemoryAddress& memAddr) const;

        std::shared_ptr<ircode::Variable> createVariable(const ircode::MemoryAddress& memAddress, ircode::Hash hash, size_t size);
    };
};