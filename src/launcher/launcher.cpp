#include <iostream>
#include <Module.hpp>
#include <Database.hpp>

int main(int argc, char *argv[])
{
    // for third party plugin only!
    std::shared_ptr<IModule> module;
    {
        auto module = GetModule("database");
        
        // use loaded module
        std::cout << "module name: " << module->getName() << std::endl;
        module->doSomething();
    }

    std::cout << "-----" << std::endl;

    // another way
    {
        auto module = DatabaseModule::Create();
        std::cout << "module name: " << module->getName() << std::endl;
        module->doSomething();
    }
    return 0;
}