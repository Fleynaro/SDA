#pragma once
#include "Core/Context.h"

namespace sda
{
    class Program;

    class Project
    {
        Program* m_program;
        std::string m_name;
        std::map<std::string, std::unique_ptr<IContext>> m_contexts;
    public:
        Project(Program* program, const std::string& name);

        // Get name of the project
        std::string getName();

        // Get context of the project
        IContext* getContext(const std::string& name);

        // Register a context
        void registerContext(std::unique_ptr<IContext> context);
    };
};