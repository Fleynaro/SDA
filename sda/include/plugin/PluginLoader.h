#pragma once
#include "plugin/PluginAPI.h"

namespace sda
{
    // Get a third-party module by path (name)
    std::shared_ptr<IPlugin> GetModule(const boost::dll::fs::path& path);
};