#pragma once
#include <Core.hpp>
#include <Module.hpp>
#include <iostream>

#include <boost/dll/alias.hpp>

class DatabaseModule : public IModule
{
public:
    std::string getName();

    void doSomething() {
        std::cout << "Database Module doSomething called" << std::endl;
        core();
    }

    static std::shared_ptr<DatabaseModule> Create() {
        return std::make_shared<DatabaseModule>();
    }
};

BOOST_DLL_ALIAS(DatabaseModule::Create, createDatabaseModule)