#pragma once
#include <list>

namespace sda::ircode
{
    class Operation;

    class Value
    {
        std::list<Operation*> m_operations;
    public:

    };

    class Constant : public Value
    {
    public:
        
    };

    class Register : public Value
    {
    public:
        
    };

    class Variable : public Value
    {
    public:
        
    };
};