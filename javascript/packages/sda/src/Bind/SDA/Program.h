#pragma once
#include "SDA/Program.h"

namespace sda::bind
{
    class ProgramBind
    {
        class RefCallbacks : public Program::Callbacks {
            void onProjectAdded(Project* project) override {
                ExportObjectRef(project);
            }

            void onProjectRemoved(Project* project) override {
                RemoveObjectRef(project);
            }
        };
        static auto New() {
            auto program = new Program();
            program->setCallbacks(std::make_shared<RefCallbacks>());
            return ExportObject(program);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<Program>(module);
            cl
                .property("projects", [](Program& self) { return to_v8(self.getProjects()); })
                .method("removeProject", &Program::removeProject)
                .static_method("New", &New);
            module.class_("Program", cl);
        }
    };
};