#pragma once
#include <boost/dll/alias.hpp>
#include <boost/dll/runtime_symbol_info.hpp>

namespace sda
{
    class Program;
    class Project;

    class IPlugin {
    public:
        // Get name of the module
        virtual std::string getName() = 0;

        // Get location of the module
        virtual boost::dll::fs::path location() const = 0;

        // Plugin loading callback
        virtual void onPluginLoaded(Program* program) = 0;

        // Project creation callback
        virtual void onProjectCreated(Project* project) = 0;
    };

    // Define exported module
    #define EXPORT_PLUGIN(plugin_name) \
        BOOST_DLL_ALIAS(plugin_name::Create, CreatePlugin)
};