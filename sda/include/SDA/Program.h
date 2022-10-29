#pragma once
#include <list>
#include "SDA/Project.h"
#include "SDA/Plugin/PluginAPI.h"

namespace sda
{
    class Program
    {
        // for access to Program::m_projects
        friend Project::Project(Program* program, const std::filesystem::path& path, std::shared_ptr<Context> context);

        std::list<std::unique_ptr<Project>> m_projects;
        std::map<std::string, std::unique_ptr<IPlugin>> m_plugins;
    public:
        Program();

        // Get list of projects
        const std::list<std::unique_ptr<Project>>& getProjects();

        void removeProject(Project* project);

        // Get plugin by name
        IPlugin* getPlugin(const std::string& name);

        // Add a plugin
        void addPlugin(std::unique_ptr<IPlugin> plugin);

        class Callbacks {
        public:
            virtual void onProjectAdded(Project* project) = 0;

            virtual void onProjectRemoved(Project* project) = 0;
        };

        void setCallbacks(std::shared_ptr<Callbacks> callbacks);

        std::shared_ptr<Callbacks> getCallbacks() const;

    private:
        std::shared_ptr<Callbacks> m_callbacks;
    };
};