#include "MyPlugin/MyPlugin.h"
#include <iostream>

using namespace sda;

std::string MyPlugin::getName() {
    return "MyPlugin";
}

boost::dll::fs::path MyPlugin::location() const {
    return boost::dll::this_line_location();
}

void MyPlugin::onPluginLoaded(Program* program) {
    // Do something
    std::cout << "MyPlugin::onPluginLoaded" << std::endl;
}

void MyPlugin::onProjectCreated(Project* project) {
    // Do something
    std::cout << "MyPlugin::onProjectCreated" << std::endl;
}

std::unique_ptr<IPlugin> MyPlugin::Create() {
    return std::make_unique<MyPlugin>();
}