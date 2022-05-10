#include "Program.h"

using namespace sda;

Project::Project(Program* program, const std::string& name)
    : m_program(program), m_name(name)
{
    program->m_projects.push_back(std::unique_ptr<Project>(this));
}

std::string Project::getName() {
    return m_name;
}

IContext* Project::getContext(const std::string& name) {
    return m_contexts.at(name).get();
}

void Project::registerContext(std::unique_ptr<IContext> context) {
    
}