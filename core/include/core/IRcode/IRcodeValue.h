#pragma once
#include <list>
#include <memory>
#include "Core/Pcode/PcodeVarnodes.h"
#include "IRcodeLinearExpr.h"

namespace sda::ircode
{
    class Operation;
    using Hash = size_t;

    class Value
    {
        Hash m_hash;
        std::list<Operation*> m_operations;
        LinearExpression m_linearExpr;
    public:
        Value(Hash hash);

        enum Type {
            Constant,
            Register,
            Variable
        };

        virtual Type getType() const = 0;

        Hash getHash() const;

        virtual size_t getSize() const = 0;

        std::list<Operation*>& getOperations();

        LinearExpression& getLinearExpr();
    };

    class Constant : public Value
    {
        const pcode::ConstantVarnode* m_constVarnode;
    public:
        Constant(const pcode::ConstantVarnode* constVarnode, Hash hash);

        Type getType() const override;

        const pcode::ConstantVarnode* getConstVarnode() const;

        size_t getSize() const override;
    };

    class Register : public Value
    {
        const pcode::RegisterVarnode* m_regVarnode;
    public:
        Register(const pcode::RegisterVarnode* regVarnode, Hash hash);

        Type getType() const override;

        const pcode::RegisterVarnode* getRegVarnode() const;

        size_t getSize() const override;
    };

    struct MemoryAddress {
        std::shared_ptr<Value> base = nullptr;
        size_t offset = 0;
    };

    class Variable : public Value
    {
        MemoryAddress m_memAddress;
        size_t m_size;
    public:
        Variable(const MemoryAddress& memAddress, Hash hash, size_t size);

        Type getType() const override;

        const MemoryAddress& getMemAddress() const;

        size_t getSize() const override;
    };
};