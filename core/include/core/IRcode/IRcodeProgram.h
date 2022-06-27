#pragma once
#include "IRcodeFunction.h"

namespace sda::ircode
{
    class Program
    {
        std::map<pcode::FunctionGraph*, Function> m_functions;
    public:
        Program() = default;

        std::map<pcode::FunctionGraph*, Function>& getFunctions();
    };
};