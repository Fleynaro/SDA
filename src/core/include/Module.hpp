#pragma once
#include <string>
#include <boost/dll/alias.hpp>
#include <boost/dll/runtime_symbol_info.hpp>

class IModule
{
public:
    // Get name of the module
    virtual std::string getName() = 0;

    // Get location of the module
    virtual boost::dll::fs::path location() const = 0;

    virtual void doSomething() = 0;
};

// Define exported module
#define EXPORT_MODULE(module_name) \
    BOOST_DLL_ALIAS(module_name::Create, CreateModule)

// Get a third-party module by path (name)
std::shared_ptr<IModule> GetModule(const boost::dll::fs::path& path);