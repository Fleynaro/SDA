#pragma once
#include "SDA/Core/IRcode/IRcodeDataTypeProvider.h"
#include "SDA/Core/IRcode/IRcodeBlock.h"
#include "SDA/Core/Pcode/PcodeInstruction.h"

namespace sda::ircode
{
    class IRcodeGenerator
    {
        Block* m_block;
        ircode::DataTypeProvider* m_dataTypeProvider;
        std::function<size_t()> m_nextVarIdProvider;
        const pcode::Instruction* m_curInstr = nullptr;
        std::set<std::shared_ptr<ircode::Variable>> m_overwrittenVariables;
        std::list<ircode::Operation*> m_genOperations;
    public:
        IRcodeGenerator(
            Block* block,
            ircode::DataTypeProvider* dataTypeProvider,
            std::function<size_t()> nextVarIdProvider);

        void ingestPcode(const pcode::Instruction* instr);

        const std::list<ircode::Operation*>& getGeneratedOperations() const;

    private:
        void genWriteMemory(MemorySubspace* memSpace, std::shared_ptr<ircode::Variable> variable);

        struct VariableReadInfo {
            std::shared_ptr<ircode::Variable> variable;
            Offset offset;
            Block* srcBlock = nullptr;

            bool operator==(const VariableReadInfo& other) const {
                return variable == other.variable && offset == other.offset;
            }
        };

        std::list<VariableReadInfo> genReadMemory(
            MemorySubspace* memSpace,
            Offset readOffset,
            size_t readSize,
            utils::BitMask& readMask);

        struct BlockReadContext {
            struct Cache {
                std::list<VariableReadInfo> varReadInfos;
                utils::BitMask readMask = 0;
            };
            const MemoryAddress& memAddr;
            size_t readSize;
            std::shared_ptr<ircode::Variable> backgroundValue;
            utils::BitSet visitedBlocks;
            std::map<Hash, Cache> cache;
        };
        std::list<VariableReadInfo> genReadMemory(Block* block,  utils::BitMask& readMask, BlockReadContext& ctx);

        std::shared_ptr<ircode::Variable> genLoadBackgroundValue(BlockReadContext& ctx);

        std::list<VariableReadInfo> genReadMemory(
            const MemoryAddress& memAddr,
            size_t readSize,
            utils::BitMask& readMask);

        std::shared_ptr<ircode::Variable> joinVariables(std::list<VariableReadInfo> varReadInfos, size_t size);

        ircode::MemoryAddress getRegisterMemoryAddress(std::shared_ptr<pcode::RegisterVarnode> regVarnode) const;

        ircode::MemoryAddress getMemoryAddress(std::shared_ptr<ircode::Value> addrValue) const;

        std::list<VariableReadInfo> genReadRegisterVarnode(std::shared_ptr<pcode::RegisterVarnode> regVarnode);

        std::shared_ptr<ircode::Value> genReadVarnode(std::shared_ptr<pcode::Varnode> varnode);

        void genOperation(std::unique_ptr<ircode::Operation> operation);

        void genGenericOperation(const pcode::Instruction* instr, ircode::OperationId operationId, ircode::MemoryAddress& outputMemAddr);

        std::shared_ptr<ircode::Variable> genLoadOperation(const ircode::MemoryAddress& memAddr, size_t loadSize);

        std::shared_ptr<ircode::Constant> createConstant(std::shared_ptr<pcode::ConstantVarnode> constVarnode) const;

        std::shared_ptr<ircode::Register> createRegister(std::shared_ptr<pcode::RegisterVarnode> regVarnode, const ircode::MemoryAddress& memAddr) const;

        std::shared_ptr<ircode::Variable> createVariable(const ircode::MemoryAddress& memAddress, ircode::Hash hash, size_t size);

        MemorySpace* getCurMemSpace() const;
    };
};