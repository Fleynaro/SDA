#include <iostream>
#include <DatabaseModule.hpp>

int main(int argc, char *argv[])
{
    // for third party plugin only!
    {
        auto module = sda::GetModule("database");
        
        // use loaded module
        std::cout << "module name: " << module->getName() << std::endl;
    }

    std::cout << "-----" << std::endl;

    // another way
    {
        auto module = sda::DatabaseModule::Create();
        std::cout << "module name: " << module->getName() << std::endl;
    }

    sda::Program program;
    program.addModule("database", sda::DatabaseModule::Create());
    return 0;
}