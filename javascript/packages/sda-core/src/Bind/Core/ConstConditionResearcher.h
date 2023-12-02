#pragma once
#include "SDA/Core/Researchers/ConstConditionResearcher.h"

using namespace sda::researcher;

namespace sda::bind
{
    class ConstConditionRepositoryBind
    {
        static auto New(ircode::Program* program) {
            auto repo = new ConstConditionRepository(program);
            return ExportObject(repo);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<ConstConditionRepository>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("eventPipe", &ConstConditionRepository::getEventPipe)
                .static_method("New", &New);
            RegisterClassName(cl, "ConstConditionRepository");
            module.class_("ConstConditionRepository", cl);
        }
    };

    static void PrintConditionsForFunctionInit(v8pp::module& module) {
        module.function("PrintConditionsForFunction", &PrintConditionsForFunction);
    }

    static void ConstConditionBindInit(v8pp::module& module) {
        ConstConditionRepositoryBind::Init(module);
        PrintConditionsForFunctionInit(module);
    }
};