#pragma once
#include <boost/dll/alias.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include "core/Program.h"

namespace sda
{
    class IPlugin {
    public:
        // Get name of the module
        virtual std::string getName() = 0;

        // Get location of the module
        virtual boost::dll::fs::path location() const = 0;

        // Initialization callback
        virtual void init(Program* program) = 0;
    };

    // Define exported module
    #define EXPORT_MODULE(module_name) \
        BOOST_DLL_ALIAS(module_name::Create, CreateModule)
};