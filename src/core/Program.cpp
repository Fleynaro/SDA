#include <Program.hpp>
#include <Module.hpp>

using namespace sda;

IModulePtr Program::getModule(const std::string& name) {
    return m_modules.at(name);
}

void Program::addModule(const std::string& name, IModulePtr module) {
    m_modules.insert(std::make_pair(name, module));
    module->init(this);
}