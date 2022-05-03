#pragma once
#include <string>
#include <boost/dll/alias.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <Program.hpp>
#include <Utils.hpp>

namespace sda
{
    class IModule {
    public:
        // Get name of the module
        virtual std::string getName() = 0;

        // Get location of the module
        virtual boost::dll::fs::path location() const = 0;

        // Initialization callback
        virtual void init(Program* program) = 0;
    };
    DECL_SHARED_PTR(IModule)

    // Define exported module
    #define EXPORT_MODULE(module_name) \
        BOOST_DLL_ALIAS(module_name::Create, CreateModule)

    // Get a third-party module by path (name)
    IModulePtr GetModule(const boost::dll::fs::path& path);
};