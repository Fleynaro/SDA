#pragma once
#include <list>
#include "Core/Pcode/PcodeVarnodes.h"

namespace sda::ircode
{
    class Operation;

    class Value
    {
        std::list<Operation*> m_operations;
    public:

        std::list<Operation*>& getOperations();
    };

    class Constant : public Value
    {
        pcode::ConstantVarnode* m_constVarnode;
    public:
        Constant() = default;

        pcode::ConstantVarnode* getConstVarnode() const;
    };

    class Register : public Value
    {
        pcode::RegisterVarnode* m_regVarnode;
    public:
        Register() = default;

        pcode::RegisterVarnode* getRegVarnode() const;
    };

    class Variable : public Value
    {
        Value* m_addressValue;
    public:
        Variable() = default;

        Value* getAddressValue() const;
    };
};