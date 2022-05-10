#pragma once
#include <list>
#include "Project.h"

namespace sda
{
    class Program
    {
        // for access to Program::m_projects
        friend Project::Project(Program* program, const std::string& name);

        std::list<std::unique_ptr<Project>> m_projects;
    public:
        // Get list of projects
        const std::list<std::unique_ptr<Project>>& getProjects();
    };
};