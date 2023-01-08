#pragma once
#include <list>
#include <memory>
#include "IRcodeLinearExpr.h"
#include "SDA/Core/Pcode/PcodeVarnodes.h"
#include "SDA/Core/DataType/DataType.h"
#include "SDA/Core/SymbolTable/SymbolTable.h"

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

        const std::list<Operation*>& getOperations() const;

        void addOperation(Operation* operation);

        const LinearExpression& getLinearExpr() const;

        void setLinearExpr(const LinearExpression& linearExpr);
    };

    class Constant : public Value
    {
        std::shared_ptr<pcode::ConstantVarnode> m_constVarnode;
    public:
        Constant(std::shared_ptr<pcode::ConstantVarnode> constVarnode, Hash hash);

        Type getType() const override;

        std::shared_ptr<pcode::ConstantVarnode> getConstVarnode() const;

        size_t getSize() const override;
    };

    class Register : public Value
    {
        std::shared_ptr<pcode::RegisterVarnode> m_regVarnode;
    public:
        Register(std::shared_ptr<pcode::RegisterVarnode> regVarnode, Hash hash);

        Type getType() const override;

        std::shared_ptr<pcode::RegisterVarnode> getRegVarnode() const;

        const sda::Register& getRegister() const;

        size_t getSize() const override;
    };

    struct MemoryAddress {
        std::shared_ptr<Value> value = nullptr;
        Hash baseAddrHash = 0; // this hash can be different from the hash of the value!
        Offset offset = 0;
    };

    class Variable : public Value
    {
        size_t m_id;
        MemoryAddress m_memAddress;
        size_t m_size;
    public:
        Variable(size_t id, const MemoryAddress& memAddress, Hash hash, size_t size);

        std::string getName() const;

        Type getType() const override;

        const MemoryAddress& getMemAddress() const;

        size_t getSize() const override;
    };
};