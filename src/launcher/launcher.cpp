#include <iostream>
#include <Module.hpp>
#include <Database.hpp>

#include <boost/dll/import.hpp>
#include <boost/function.hpp>

int main(int argc, char *argv[])
{
    printHelloWorld();

    // ЛИНУКС ЧЕКНУТЬ! + СДЕЛАТЬ ADVANCED ЧАСТЬ!

    // for third party plugin only!
    {
        std::shared_ptr<IModule> module;

        // find module
        boost::dll::fs::path shared_library_path("database");
        typedef std::shared_ptr<IModule> (moduleapi_create_t)();
        boost::function<moduleapi_create_t> moduleCreator;
        moduleCreator = boost::dll::import_alias<moduleapi_create_t>(
            shared_library_path,
            "createDatabaseModule",
            boost::dll::load_mode::append_decorations);
        module = moduleCreator();
        
        // use loaded module
        std::cout << "module name: " << module->getName() << std::endl;
        module->doSomething();
    }

    // another way
    {
        auto module = DatabaseModule::Create();
        std::cout << "module name: " << module->getName() << std::endl;
        module->doSomething();
    }

    system("pause");
    return 0;
}