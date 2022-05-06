#include <iostream>
#include <CoreModule.hpp>
#include <DatabaseModule.hpp>

int main(int argc, char *argv[])
{
    using namespace sda;

    Program program;
    program.addModule(CoreModule::Create());
    program.addModule(DatabaseModule::Create());

    auto project = Project::Create(&program, "Test");

    return 0;
}