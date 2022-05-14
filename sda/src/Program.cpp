#include "Program.h"
#include "Factory.h"

using namespace sda;

Program::Program() {
    m_factory = std::make_unique<Factory>();
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

Factory* Program::getFactory() {
    return m_factory.get();
}