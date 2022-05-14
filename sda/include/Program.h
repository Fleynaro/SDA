#pragma once
#include <list>
#include "Project.h"
#include "Plugin/PluginAPI.h"

namespace sda
{
    class Factory;

    class Program
    {
        // for access to Program::m_projects
        friend Project::Project(Program* program, const std::filesystem::path& path, Context* context);

        std::list<std::unique_ptr<Project>> m_projects;
        std::map<std::string, std::unique_ptr<IPlugin>> m_plugins;
        std::unique_ptr<Factory> m_factory;
    public:
        Program();

        // Get list of projects
        const std::list<std::unique_ptr<Project>>& getProjects();

        // Get plugin by name
        IPlugin* getPlugin(const std::string& name);

        // Get factory
        Factory* getFactory();
    };
};