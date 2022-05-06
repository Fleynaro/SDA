#include <Program.hpp>
#include <Module.hpp>

using namespace sda;

IModulePtr Program::getModule(const std::string& name) {
    return m_modules.at(name);
}

void Program::addModule(IModulePtr module) {
    m_modules[module->getName()] = module;
}

const std::list<ProjectPtr>& Program::getProjects() {
    return m_projects;
}