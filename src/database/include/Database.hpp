#pragma once
#include <Core.hpp>
#include <Module.hpp>
#include <iostream>

class DatabaseModule : public IModule
{
public:
    std::string getName();

    boost::dll::fs::path location() const override {
        return boost::dll::this_line_location();
    }

    void doSomething() {
        std::cout << "Database Module doSomething called" << std::endl;
        core();
    }

    static std::unique_ptr<IModule> Create() {
        return std::make_unique<DatabaseModule>();
    }
};

EXPORT_MODULE(DatabaseModule)