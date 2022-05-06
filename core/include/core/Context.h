#pragma once
#include <string>
#include <memory>

namespace sda
{
    class IContext
    {
    public:
        // Get name of the module
        virtual std::string getName() = 0;


    };
};