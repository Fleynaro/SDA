#pragma once
#include "plugin/PluginAPI.h"

namespace sda
{
    // Get a third-party module by path (name)
    std::shared_ptr<IPlugin> GetPlugin(const boost::dll::fs::path& path);

    // Get all third-party modules in a directory
    std::list<std::shared_ptr<IPlugin>> GetPlugins(const boost::dll::fs::path& path);
};