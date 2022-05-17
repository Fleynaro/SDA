#pragma once
#include "Plugin/PluginAPI.h"

namespace sda
{
    class MyPlugin : public IPlugin
    {
    public:
        // Get name of the module
        std::string getName() override;

        // Get location of the module
        boost::dll::fs::path location() const override;

        // Plugin loading callback
        void onPluginLoaded(Program* program) override;

        // Project creation callback
        void onProjectCreated(Project* project) override;

        // Create plugin instance
        static std::unique_ptr<IPlugin> Create();
    };

    // Export plugin
    EXPORT_PLUGIN(MyPlugin)
};