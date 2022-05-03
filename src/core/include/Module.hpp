#pragma once
#include <string>

class IModule
{
public:
    virtual std::string getName() = 0;

    virtual void doSomething() = 0;
};

void printHelloWorld();