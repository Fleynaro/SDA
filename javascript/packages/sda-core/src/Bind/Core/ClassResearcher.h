#pragma once
#include "SDA/Core/Researchers/ClassResearcher.h"

using namespace sda::researcher;

namespace sda::bind
{
    class FieldStructureGroupBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<FieldStructureGroup>(module);
            cl
                .auto_wrap_objects(true)
                .auto_wrap_object_ptrs(true)
                .property("structures", &FieldStructureGroup::getStructures);
            RegisterClassName(cl, "FieldStructureGroup");
            module.class_("FieldStructureGroup", cl);
        }
    };

    class StructureInfoBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<StructureInfo>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("labelOffset", &StructureInfo::getLabelOffset)
                .property("labels", &StructureInfo::getLabels)
                .property("labelSet", &StructureInfo::getLabelSet)
                .property("parents", &StructureInfo::getParents)
                .property("childs", &StructureInfo::getChilds)
                .property("inputs", &StructureInfo::getInputs)
                .property("outputs", &StructureInfo::getOutputs)
                .property("group", &StructureInfo::getGroup);
            RegisterClassName(cl, "StructureInfo");
            module.class_("StructureInfo", cl);
        }
    };

    class ClassRepositoryBind
    {
        static auto New(std::shared_ptr<EventPipe> eventPipe) {
            auto repo = new ClassRepository(eventPipe);
            return ExportObject(repo);
        }

        static void AddUserDefinedLabelOffset(ClassRepository* repo, Structure* structure, v8::Local<v8::Object> jsInfo) {
            auto isolate = v8::Isolate::GetCurrent();
            ClassLabelInfo info;
            v8pp::get_option(isolate, jsInfo, "structureInstrOffset", info.structureInstrOffset);
            v8pp::get_option(isolate, jsInfo, "sourceInstrOffset", info.sourceInstrOffset);
            v8pp::get_option(isolate, jsInfo, "labelOffset", info.labelOffset);
            repo->addUserDefinedLabelOffset(structure, info);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<ClassRepository>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .method("addUserDefinedLabelOffset", &AddUserDefinedLabelOffset)
                .method("getStructureInfo", &ClassRepository::getStructureInfo)
                .static_method("New", &New);
            RegisterClassName(cl, "ClassRepository");
            module.class_("ClassRepository", cl);
        }
    };

    class ClassResearcherBind
    {
        static auto New(
            ircode::Program* program,
            ClassRepository* classRepo,
            StructureRepository* structureRepo
        ) {
            auto researcher = new ClassResearcher(
                program,
                classRepo,
                structureRepo
            );
            return ExportObject(researcher);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<ClassResearcher>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("eventPipe", &ClassResearcher::getEventPipe)
                .static_method("New", &New);
            RegisterClassName(cl, "ClassResearcher");
            module.class_("ClassResearcher", cl);
        }
    };

    static void ClassBindInit(v8pp::module& module) {
        FieldStructureGroupBind::Init(module);
        StructureInfoBind::Init(module);
        ClassRepositoryBind::Init(module);
        ClassResearcherBind::Init(module);
    }
};