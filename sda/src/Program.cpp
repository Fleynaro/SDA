#include "Program.h"
#include "Factory.h"

using namespace sda;

Program::Program() {
}

const std::list<std::unique_ptr<Project>>& Program::getProjects() {
    return m_projects;
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