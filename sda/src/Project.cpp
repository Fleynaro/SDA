#include "Program.h"

using namespace sda;

Project::Project(Program* program, const std::string& name, Context* context)
    : m_program(program), m_name(name), m_context(context)
{
    program->m_projects.push_back(std::unique_ptr<Project>(this));
}

std::string Project::getName() {
    return m_name;
}

Context* Project::getContext() {
    return m_context;
}

ContextCallbacks* Project::getContextCallbacks() {
    return dynamic_cast<ContextCallbacks*>(m_context->getCallbacks());
}