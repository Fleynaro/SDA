#pragma once
#include "SDA/Program.h"

namespace sda::bind
{
    class ProgramBind
    {
        static auto New() {
            auto program = new Program();
            return ExportObject(program);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<Program>(module);
            cl
                .property("projects", [](Program& self) { return to_v8(self.getProjects()); })
                .static_method("New", &New);
            module.class_("Program", cl);
        }
    };
};