#pragma once
#include "SDA/Core/Researchers/DataFlowResearcher.h"

using namespace sda::researcher;

namespace sda::bind
{
    class DataFlowNodeBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<DataFlowNode>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("type", &DataFlowNode::getType)
                .property("name", [](DataFlowNode& self) { return self.getName(true); })
                .property("variable", &DataFlowNode::getVariable)
                .property("constant", &DataFlowNode::getConstant)
                .property("predecessors", &DataFlowNode::getPredecessors)
                .property("successors", &DataFlowNode::getSuccessors);
            RegisterClassName(cl, "DataFlowNode");
            module.class_("DataFlowNode", cl);
        }
    };

    class DataFlowRepositoryBind
    {
        static auto New(std::shared_ptr<EventPipe> eventPipe) {
            auto repo = new DataFlowRepository(eventPipe);
            return ExportObject(repo);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<DataFlowRepository>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("globalStartNode", &DataFlowRepository::getGlobalStartNode)
                .method("getNode", &DataFlowRepository::getNode)
                .static_method("New", &New);
            RegisterClassName(cl, "DataFlowRepository");
            module.class_("DataFlowRepository", cl);
        }
    };

    class DataFlowCollectorBind
    {
        static auto New(ircode::Program* program,DataFlowRepository* dataFlowRepo) {
            auto researcher = new DataFlowCollector(program, dataFlowRepo);
            return ExportObject(researcher);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<DataFlowCollector>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("eventPipe", &DataFlowCollector::getEventPipe)
                .static_method("New", &New);
            RegisterClassName(cl, "DataFlowCollector");
            module.class_("DataFlowCollector", cl);
        }
    };

    static void PrintDataFlowForFunctionInit(v8pp::module& module) {
        module.function("PrintDataFlowForFunction", &PrintDataFlowForFunction);
    }

    static void DataFlowBindInit(v8pp::module& module) {
        DataFlowNodeBind::Init(module);
        DataFlowRepositoryBind::Init(module);
        DataFlowCollectorBind::Init(module);
        PrintDataFlowForFunctionInit(module);
    }
};