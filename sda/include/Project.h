#pragma once
#include "Core/Context.h"
#include "Callbacks/ContextCallbacks.h"

namespace sda
{
    class Program;

    class Project
    {
        Program* m_program;
        std::string m_name;
        Context* m_context;
    public:
        Project(Program* program, const std::string& name, Context* context);

        // Get name of the project
        std::string getName();

        // Get context of the project
        Context* getContext();

        // Get context callbacks
        ContextCallbacks* getContextCallbacks();
    };
};