#pragma once
#include "SDA/Project.h"

namespace sda::bind
{
    class ProjectBind
    {
        static auto New(Program* program, const std::filesystem::path& path, std::shared_ptr<Context> context) {
            return new Project(program, path, context);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<Project>(module);
            cl
                .property("program", &Project::getProgram)
                .property("path", &Project::getPath)
                .property("context", &Project::getContext)
                .static_method("New", &New);
            module.class_("Project", cl);
        }
    };
};