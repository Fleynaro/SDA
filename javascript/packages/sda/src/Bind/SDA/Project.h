#pragma once
#include "SDA/Project.h"
#include "SDA/Callbacks/ProjectContextCallbacks.h"

namespace sda::bind
{
    class ProjectBind
    {
        static auto New(Program* program, const std::filesystem::path& path, std::unique_ptr<Context> context) {
            return new Project(program, path, std::move(context));
        }
        
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<Project>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("program", &Project::getProgram)
                .property("path", &Project::getPath)
                .property("context", &Project::getContext)
                .property("canBeSaved", &Project::canBeSaved)
                .method("load", &Project::load)
                .method("save", &Project::save)
                .static_method("New", &New);
            ObjectLookupTableRaw::Register(cl);
            RegisterClassName(cl, "Project");
            module.class_("Project", cl);
        }
    };

    class ProjectContextCallbacksBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<ProjectContextCallbacks, v8pp::shared_ptr_traits>(module);
            cl
                .inherit<Context::Callbacks>()
                .method("setTransactionEnabled", &ProjectContextCallbacks::setTransactionEnabled)
                .method("setChangeEnabled", &ProjectContextCallbacks::setChangeEnabled);
        }
    };
};