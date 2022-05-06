#pragma once
#include <map>
#include "Context.h"

namespace sda
{
    class Program;

    class Project
    {
        Program* m_program;
        std::string m_name;
        std::map<std::string, std::unique_ptr<IContext>> m_contexts;

        Project(Program* program, const std::string& name);
    public:
        // Get name of the project
        std::string getName();

        // Get context of the project
        IContext* getContext(const std::string& name);

        // Create a new project
        static Project* Create(Program* program, const std::string& name);
    };
};