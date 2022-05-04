#pragma once
#include <map>
#include <Context.hpp>

namespace sda
{
    DECL_SHARED_PTR_BEFORE(Program);
    DECL_UNIQUE_PTR_BEFORE(Project);
    class Project
    {
        ProgramPtr m_program;
        std::string m_name;
        std::map<std::string, IContextPtr> m_contexts;

        Project(ProgramPtr program, const std::string& name);
    public:
        // Get name of the project
        std::string getName();

        // Get context of the project
        IContextPtr getContext(const std::string& name);

        // Add context to the project
        void addContext(IContextPtr context);

        // Create a new project
        static Project* Create(Program* program, const std::string& name);
    };
};