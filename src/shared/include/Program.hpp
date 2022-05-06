#pragma once
#include <list>
#include <map>
#include <Project.hpp>

namespace sda
{
    DECL_SHARED_PTR_BEFORE(IModule);

    class Program
    {
        // for access to Program::m_projects
        friend Project* Project::Create(Program* program, const std::string& name);

        std::map<std::string, IModulePtr> m_modules;
        std::list<ProjectPtr> m_projects;
    public:
        // Add module to the program
        void addModule(IModulePtr module);

        // Get module by name
        IModulePtr getModule(const std::string& name);

        // Get list of projects
        const std::list<ProjectPtr>& getProjects();
    };
};