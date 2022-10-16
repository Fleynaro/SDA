#pragma once
#include "SDA/Core/IRcode/IRcodeFunction.h"
#include "SDA/Core/Pcode/PcodeFunctionGraph.h"

namespace sda::decompiler
{
    class IRcodeFunctionGenerator
    {
        ircode::Function* m_function;
        pcode::FunctionGraph* m_functionGraph;
    public:
        IRcodeFunctionGenerator(ircode::Function* function, pcode::FunctionGraph* functionGraph);

        void start();
    };
};