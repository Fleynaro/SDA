#pragma once
#include "Plugin/PluginAPI.h"

// TODO: разработка внутри SDA, а не отдельно!

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

        static std::unique_ptr<IPlugin> Create();
    };

    EXPORT_PLUGIN(MyPlugin)
};