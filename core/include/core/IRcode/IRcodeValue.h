#pragma once
#include <list>
#include "Core/Pcode/PcodeVarnodes.h"

namespace sda::ircode
{
    class Operation;
    using Hash = size_t;

    struct MemoryAddress {
        std::shared_ptr<Value> base = nullptr;
        size_t offset = 0;
    };

    class Value
    {
        Hash m_hash;
        std::list<Operation*> m_operations;
        
    public:
        Value(Hash hash);

        Hash getHash() const;

        virtual size_t getSize() const = 0;

        std::list<Operation*>& getOperations();
    };

    class Constant : public Value
    {
        pcode::ConstantVarnode* m_constVarnode;
    public:
        Constant(pcode::ConstantVarnode* constVarnode, Hash hash);

        pcode::ConstantVarnode* getConstVarnode() const;

        size_t getSize() const override;
    };

    class Register : public Value
    {
        pcode::RegisterVarnode* m_regVarnode;
    public:
        Register(pcode::RegisterVarnode* regVarnode, Hash hash);

        pcode::RegisterVarnode* getRegVarnode() const;

        size_t getSize() const override;
    };

    class Variable : public Value
    {
        MemoryAddress m_memAddress;
        size_t m_size;
    public:
        Variable(const MemoryAddress& memAddress, Hash hash, size_t size);

        const MemoryAddress& getMemAddress() const;

        size_t getSize() const override;
    };
};