#pragma once
#include <string>
#include <map>
#include <Utils.hpp>

namespace sda
{
    DECL_SHARED_PTR_BEFORE(IModule);

    class Program
    {
        std::map<std::string, IModulePtr> m_modules;
    public:
        // Get module by name
        IModulePtr getModule(const std::string& name);

        // Add module to the program
        void addModule(const std::string& name, IModulePtr module);
    };
};