#pragma once
#include "SDA/Core/Researchers/StructureResearcher.h"

using namespace sda::researcher;

namespace sda::bind
{
    class ConstantSetBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<ConstantSet>(module);
            cl
                .auto_wrap_objects(true)
                .auto_wrap_object_ptrs(true)
                .property("values", &ConstantSet::values);
            RegisterClassName(cl, "ConstantSet");
            module.class_("ConstantSet", cl);
        }
    };

    class StructureBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<Structure>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("name", &Structure::getName)
                .property("sourceNode", &Structure::getSourceNode)
                .property("linkedNodes", &Structure::getLinkedNodes)
                .property("parents", &Structure::getParents)
                .property("childs", &Structure::getChilds)
                .property("inputs", &Structure::getInputs)
                .property("outputs", &Structure::getOutputs)
                .property("fields", &Structure::getFields)
                .property("conditions", &Structure::getConditions)
                .property("constants", &Structure::getConstants);
            RegisterClassName(cl, "Structure");
            module.class_("Structure", cl);
        }
    };

    class StructureRepositoryBind
    {
        static auto New(std::shared_ptr<EventPipe> eventPipe) {
            auto repo = new StructureRepository(eventPipe);
            return ExportObject(repo);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<StructureRepository>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("allStructures", &StructureRepository::getAllStructures)
                .property("rootStructures", &StructureRepository::getRootStructures)
                .method("getStructure", &StructureRepository::getStructure)
                .method("getStructureByName", &StructureRepository::getStructureByName)
                .method("getLink", std::function([](StructureRepository* repo, DataFlowNode* node) -> v8::Local<v8::Value> {
                    auto isolate = v8::Isolate::GetCurrent();
                    auto link = repo->getLink(node);
                    if (!link) {
                        return v8::Undefined(isolate);
                    }
                    auto linkObj = v8::Object::New(isolate);
                    v8pp::set_option(isolate, linkObj, "structure", link->structure);
                    v8pp::set_option(isolate, linkObj, "offset", link->offset);
                    v8pp::set_option(isolate, linkObj, "own", link->own);
                    return linkObj;
                }))
                .static_method("New", &New);
            RegisterClassName(cl, "StructureRepository");
            module.class_("StructureRepository", cl);
        }
    };

    class StructureResearcherBind
    {
        static auto New(
            ircode::Program* program,
            StructureRepository* structureRepo,
            DataFlowRepository* dataFlowRepo,
            researcher::ConstConditionRepository* constCondRepo
        ) {
            auto researcher = new StructureResearcher(
                program,
                structureRepo,
                dataFlowRepo,
                constCondRepo
            );
            return ExportObject(researcher);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<StructureResearcher>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("eventPipe", &StructureResearcher::getEventPipe)
                .static_method("New", &New);
            RegisterClassName(cl, "StructureResearcher");
            module.class_("StructureResearcher", cl);
        }
    };

    static void PrintStructureInit(v8pp::module& module) {
        module.function("PrintStructure", &PrintStructureStr);
    }

    static void StructureBindInit(v8pp::module& module) {
        ConstantSetBind::Init(module);
        StructureBind::Init(module);
        StructureRepositoryBind::Init(module);
        StructureResearcherBind::Init(module);
        PrintStructureInit(module);
    }
};