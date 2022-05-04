#include <Project.hpp>
#include <Program.hpp>
#include <Module.hpp>

using namespace sda;

Project::Project(ProgramPtr program, const std::string& name)
    : m_program(program), m_name(name)
{}

std::string Project::getName() {
    return m_name;
}

IContextPtr Project::getContext(const std::string& name) {
    return m_contexts.at(name);
}

void Project::addContext(IContextPtr context) {
    m_contexts[context->getName()] = context;
}

Project* Project::Create(Program* program, const std::string& name) {
    auto project = std::make_unique<Project>(program, name);
    
    for (auto& [name, module] : program->m_modules) {
        project->addContext(module->createContext());
    }
    program->m_projects.push_back(std::move(project));

    return project.get();
}