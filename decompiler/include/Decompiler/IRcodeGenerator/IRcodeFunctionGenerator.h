#pragma once
#include "Core/IRcode/IRcodeFunction.h"
#include "Core/Pcode/PcodeFunctionGraph.h"

namespace sda::decompiler
{
    class IRcodeGenerator
    {
        ircode::Function* m_function;
        pcode::FunctionGraph* m_functionGraph;
    public:
        IRcodeGenerator(ircode::Function* function, pcode::FunctionGraph* functionGraph);

        void start();
    };
};