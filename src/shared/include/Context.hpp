#pragma once
#include <string>
#include <Utils.hpp>

namespace sda
{
    DECL_SHARED_PTR_BEFORE(IContext);
    class IContext
    {
    public:
        // Get name of the module
        virtual std::string getName() = 0;


    };
};