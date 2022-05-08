#include "Program.h"

using namespace sda;

const std::list<std::unique_ptr<Project>>& Program::getProjects() {
    return m_projects;
}