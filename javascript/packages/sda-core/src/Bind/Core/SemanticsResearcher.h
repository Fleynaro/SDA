#pragma once
#include "SDA/Core/Researchers/SemanticsResearcher.h"

using namespace sda::researcher;

namespace sda::bind
{
    class SemanticsBind
    {
        static void InitAbstract(v8pp::module& module) {
            auto cl = NewClass<Semantics>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("name", &Semantics::toString)
                .property("hash", std::function([](Semantics& self) {
                    return std::to_string(self.getHash());
                }))
                .property("successors", &Semantics::getSuccessors)
                .property("predecessors", &Semantics::getPredecessors);
            RegisterClassName(cl, "Semantics");
            module.class_("Semantics", cl);
        }

        static void InitHolder(v8pp::module& module) {
            auto cl = NewClass<HolderSemantics>(module);
            cl
                .inherit<Semantics>()
                .property("holder", &HolderSemantics::getHolder)
                .property("semantics", &HolderSemantics::getSemantics);
            RegisterClassName(cl, "HolderSemantics");
            module.class_("HolderSemantics", cl);
        }
    public:
        static void Init(v8pp::module& module) {
            InitAbstract(module);
            InitHolder(module);
        }
    };

    class SemanticsObjectBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<SemanticsObject>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("semantics", &SemanticsObject::getSemantics)
                .property("variables", &SemanticsObject::getVariables);
            RegisterClassName(cl, "SemanticsObject");
            module.class_("SemanticsObject", cl);
        }
    };

    class SemanticsPropagatorBind
    {
        static void InitAbstract(v8pp::module& module) {
            auto cl = NewClass<SemanticsPropagator>(module);
            cl
                .auto_wrap_object_ptrs(true);
            RegisterClassName(cl, "SemanticsPropagator");
            module.class_("SemanticsPropagator", cl);
        }

        static auto NewBase(
            ircode::Program* program,
            SemanticsRepository* repository,
            DataFlowRepository* dataFlowRepo
        ) {
            return new BaseSemanticsPropagator(program, repository, dataFlowRepo);
        }

        static void InitBase(v8pp::module& module) {
            auto cl = NewClass<BaseSemanticsPropagator>(module);
            cl
                .inherit<SemanticsPropagator>()
                .static_method("New", &NewBase);
            RegisterClassName(cl, "BaseSemanticsPropagator");
            module.class_("BaseSemanticsPropagator", cl);
        }
    public:
        static void Init(v8pp::module& module) {
            InitAbstract(module);
            InitBase(module);
        }
    };

    class SemanticsRepositoryBind
    {
        static auto New(std::shared_ptr<EventPipe> eventPipe) {
            auto repo = new SemanticsRepository(eventPipe);
            return ExportObject(repo);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<SemanticsRepository>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("allObjects", &SemanticsRepository::getObjects)
                .method("getObject", &SemanticsRepository::getObject)
                .method("getSemantics", std::function([](SemanticsRepository* repo, std::string hash) {
                    return repo->getSemantics(std::stoull(hash));
                }))
                .static_method("New", &New);
            RegisterClassName(cl, "SemanticsRepository");
            module.class_("SemanticsRepository", cl);
        }
    };

    class SemanticsResearcherBind
    {
        static auto New(
            ircode::Program* program,
            SemanticsRepository* semanticsRepo,
            ClassRepository* classRepo,
            DataFlowRepository* dataFlowRepo
        ) {
            auto researcher = new SemanticsResearcher(
                program,
                semanticsRepo,
                classRepo,
                dataFlowRepo
            );
            return ExportObject(researcher);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<SemanticsResearcher>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("eventPipe", &SemanticsResearcher::getEventPipe)
                .method("addPropagator", &SemanticsResearcher::addPropagator)
                .static_method("New", &New);
            RegisterClassName(cl, "SemanticsResearcher");
            module.class_("SemanticsResearcher", cl);
        }
    };

    static void SemanticsBindInit(v8pp::module& module) {
        SemanticsBind::Init(module);
        SemanticsObjectBind::Init(module);
        SemanticsPropagatorBind::Init(module);
        SemanticsRepositoryBind::Init(module);
        SemanticsResearcherBind::Init(module);
    }
};