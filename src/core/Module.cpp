#include <Module.hpp>
#include <boost/function.hpp>
#include <boost/dll/import.hpp>

// Bind module object to the shared library to prevent it from being unloaded
std::shared_ptr<IModule> Bind(std::unique_ptr<IModule> module) {
    // getting location of the shared library that holds the plugin
    auto location = module->location();

    struct {
        std::shared_ptr<boost::dll::shared_library> lib;

        void operator()(IModule* p) const {
            delete p;
        }
    } deleter;
    deleter.lib = std::make_shared<boost::dll::shared_library>(location);

    return std::shared_ptr<IModule>(module.release(), deleter);
}

std::shared_ptr<IModule> GetModule(const boost::dll::fs::path& path) {
    typedef std::unique_ptr<IModule> (moduleapi_create_t)();

    /*
        Module creator variable holds a reference to the shared library (module).
        If this variable goes out of scope or will be reset, then the module will be unloaded!
        To prevent this, we use Bind() function.

        See:
        https://www.boost.org/doc/libs/1_76_0/doc/html/boost_dll/tutorial.html#boost_dll.tutorial.advanced_library_reference_counting
    */
    boost::function<moduleapi_create_t> moduleCreator;
    moduleCreator = boost::dll::import_alias<moduleapi_create_t>(
        path,
        "CreateModule",
        boost::dll::load_mode::append_decorations);

    auto module = moduleCreator();
    return Bind(std::move(module));
}