#include "SDA/Program.h"
#include "SDA/Factory.h"

using namespace sda;

Program::Program()
{
    m_callbacks = std::make_unique<Callbacks>();
}

const std::list<std::unique_ptr<Project>>& Program::getProjects() {
    return m_projects;
}

void Program::removeProject(Project* project) {
    m_callbacks->onProjectRemoved(project);
    m_projects.remove_if([project](const std::unique_ptr<Project>& p) {
        return p.get() == project;
    });
}

IPlugin* Program::getPlugin(const std::string& name) {
    auto it = m_plugins.find(name);
    if (it == m_plugins.end()) {
        return nullptr;
    }
    return it->second.get();
}

void Program::addPlugin(std::unique_ptr<IPlugin> plugin) {
    m_plugins[plugin->getName()] = std::move(plugin);
}

void Program::setCallbacks(std::shared_ptr<Callbacks> callbacks) {
    m_callbacks = callbacks;
}

std::shared_ptr<Program::Callbacks> Program::getCallbacks() const {
    return m_callbacks;
}