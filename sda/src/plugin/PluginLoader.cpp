#include "SDA/plugin/PluginLoader.h"
#include <boost/function.hpp>
#include <boost/dll/import.hpp>

using namespace sda;

// Bind plugin object to the shared library to prevent it from being unloaded
std::shared_ptr<IPlugin> Bind(std::unique_ptr<IPlugin> plugin) {
    // getting location of the shared library that holds the plugin
    auto location = plugin->location();

    struct {
        std::shared_ptr<boost::dll::shared_library> lib;

        void operator()(IPlugin* p) const {
            delete p;
        }
    } deleter;
    deleter.lib = std::make_shared<boost::dll::shared_library>(location);
    
    return std::shared_ptr<IPlugin>(plugin.release(), deleter);
}

std::shared_ptr<IPlugin> sda::GetPlugin(const boost::dll::fs::path& path) {
    typedef std::unique_ptr<IPlugin> (pluginapi_create_t)();

    /*
        Plugin creator variable holds a reference to the shared library (plugin).
        If this variable goes out of scope or will be reset, then the plugin will be unloaded!
        To prevent this, we use Bind() function.

        See:
        https://www.boost.org/doc/libs/1_76_0/doc/html/boost_dll/tutorial.html#boost_dll.tutorial.advanced_library_reference_counting
    */
    boost::dll::shared_library lib(path, boost::dll::load_mode::append_decorations);
    if (!lib.has("CreatePlugin"))
        throw std::runtime_error("Plugin does not have Create function or EXPORT_PLUGIN macro is not defined");

    boost::function<pluginapi_create_t> pluginCreator;
    pluginCreator = boost::dll::import_alias<pluginapi_create_t>(lib, "CreatePlugin");

    auto plugin = pluginCreator();
    return Bind(std::move(plugin));
}

std::list<std::shared_ptr<IPlugin>> sda::GetPlugins(const boost::dll::fs::path& path) {
    std::list<std::shared_ptr<IPlugin>> plugins;

    for (auto& file : boost::dll::fs::directory_iterator(path)) {
        if (file.path().extension() == ".dll") {
            auto plugin = GetPlugin(file.path());
            plugins.push_back(plugin);
        }
    }

    return plugins;
}