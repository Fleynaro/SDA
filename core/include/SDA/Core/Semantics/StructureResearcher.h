#pragma once
#include "DataFlowSemantics.h"
#include "ConstConditionSemantics.h"
#include "SDA/Core/Commit.h"
#include "SDA/Core/DataType/StructureDataType.h"
#include "SDA/Core/IRcode/IRcodeBlock.h"
#include "SDA/Core/IRcode/IRcodeEvents.h"
#include "SDA/Core/IRcode/IRcodeHelper.h"
#include "SDA/Core/Utils/IOManip.h"

namespace sda::semantics
{
    class ConditionSet {
        std::map<size_t, std::set<size_t>> m_conditions;
    public:
        ConditionSet() = default;

        void insert(size_t offset, size_t value) {
            m_conditions[offset].insert(value);
        }

        void merge(const ConditionSet& other, bool clear = false) {
            for (auto& [offset, values] : other.m_conditions) {
                if (clear) {
                    m_conditions[offset].clear();
                }
                m_conditions[offset].insert(values.begin(), values.end());
            }
        }

        void clear() {
            m_conditions.clear();
        }

        size_t hash() {
            size_t result = 0;
            for (auto& [offset, values] : m_conditions) {
                boost::hash_combine(result, offset);
                for (auto& value : values) {
                    boost::hash_combine(result, value);
                }
            }
            return result;
        }
    };

    struct Structure {
        std::string name;
        std::list<Structure*> parents;
        std::list<Structure*> childs;
        std::map<size_t, Structure*> fields;
        ConditionSet conditions;

        void addChild(Structure* structure) {
            childs.push_back(structure);
            structure->parents.push_back(this);
        }

        void clearParents() {
            for (auto& parent : parents) {
                parent->childs.remove(this);
            }
            parents.clear();
        }

        void clearChilds() {
            for (auto& child : childs) {
                child->parents.remove(this);
            }
            childs.clear();
        }
    };

    class StructureRepository
    {
        struct Link {
            Structure* structure;
            size_t offset;
        };
        std::list<Structure> m_structures;
        std::map<DataFlowNode*, Link> m_nodeToStructure;
    public:
        StructureRepository()
        {}

        std::list<Structure*> getRootStructures() {
            std::list<Structure*> result;
            for (auto& structure : m_structures) {
                if (structure.parents.empty()) {
                    result.push_back(&structure);
                }
            }
            return result;
        }

        Structure* createStructure(const std::string& name) {
            m_structures.emplace_back(Structure { name });
            return &m_structures.back();
        }

        Structure* getOrCreateStructure(DataFlowNode* node) {
            auto link = getLink(node);
            if (link == nullptr) {
                auto structure = createStructure(node->variable ? node->variable->getName(true) : "root");
                addLink(node, structure);
                return structure;
            }
            return link->structure;
        }

        void removeStructure(Structure* structure) {
            structure->clearParents();
            structure->clearChilds();
            m_structures.remove_if([structure](const Structure& s) {
                return &s == structure;
            });
        }

        void addField(Structure* structure, size_t offset, Structure* varStructure) {
            auto it = structure->fields.find(offset);
            if (it == structure->fields.end()) {
                auto offsetStr = (std::stringstream() << "0x" << utils::to_hex() << offset).str();
                auto fieldStructure = createStructure(structure->name + "_" + offsetStr);
                it = structure->fields.emplace(offset, fieldStructure).first;
            }
            auto fieldStructure = it->second;
            varStructure->clearParents();
            fieldStructure->addChild(varStructure);
        }

        void addLink(DataFlowNode* node, Structure* structure, size_t offset = 0) {
            m_nodeToStructure[node] = { structure, offset };
        }

        const Link* getLink(DataFlowNode* node) const {
            auto it = m_nodeToStructure.find(node);
            if (it == m_nodeToStructure.end()) {
                return nullptr;
            }
            return &it->second;
        }

        size_t getHash(DataFlowNode* node) const {
            auto link = getLink(node);
            if (link == nullptr) {
                return 0;
            }
            size_t result = reinterpret_cast<size_t>(link->structure);
            boost::hash_combine(result, link->structure->conditions.hash());
            return result;
        }
    };

    class StructureResearcher
    {
        ircode::Program* m_program;
        Platform* m_platform;
        StructureRepository* m_structureRepo;
        DataFlowRepository* m_dataFlowRepo;
        semantics::ConstConditionRepository* m_constCondRepo;

        class EventHandler
        {
            StructureResearcher* m_researcher;

            void handleDataFlowNodeUpdatedEvent(const DataFlowNodeUpdatedEvent& event) {
                m_researcher->research(event.node);
            }

            static std::shared_ptr<EventPipe> GetOptimizedEventPipe() {
                struct Data {
                    std::list<DataFlowNode*> updatedNodes;
                };
                auto data = std::make_shared<Data>();
                auto filter = EventPipe::FilterOr(
                    EventPipe::Filter(std::function([](const DataFlowNodeCreatedEvent& event) {
                        return true;
                    })),
                    EventPipe::Filter(std::function([](const DataFlowNodeUpdatedEvent& event) {
                        return true;
                    }))
                );
                auto commitEmitter = std::function([data](const EventNext& next) {
                    while (!data->updatedNodes.empty()) {
                        auto node = data->updatedNodes.front();
                        data->updatedNodes.pop_front();
                        next(DataFlowNodeUpdatedEvent(node));
                    }
                });
                std::shared_ptr<EventPipe> commitPipeIn;
                auto pipe = OptimizedCommitPipe(filter, commitPipeIn, commitEmitter);
                commitPipeIn->subscribe(std::function([data](const DataFlowNodeCreatedEvent& event) {
                    // creating event is treated as updating event
                    data->updatedNodes.push_back(event.node);
                }));
                commitPipeIn->subscribe(std::function([data](const DataFlowNodeUpdatedEvent& event) {
                    auto it = std::find(data->updatedNodes.begin(), data->updatedNodes.end(), event.node);
                    if (it == data->updatedNodes.end()) {
                        data->updatedNodes.push_back(event.node);
                    }
                }));
                return pipe;
            }
        public:
            EventHandler(StructureResearcher* researcher) : m_researcher(researcher) {}

            std::shared_ptr<EventPipe> getEventPipe() {
                auto pipe = EventPipe::New();
                pipe
                    ->connect(GetOptimizedEventPipe())
                    ->subscribeMethod(this, &EventHandler::handleDataFlowNodeUpdatedEvent);
                return pipe;
            }
        };
        EventHandler m_ircodeEventHandler;
    public:
        StructureResearcher(
            ircode::Program* program,
            Platform* platform,
            StructureRepository* structureRepo,
            DataFlowRepository* dataFlowRepo,
            semantics::ConstConditionRepository* constCondRepo
        )
            : m_program(program)
            , m_platform(platform)
            , m_structureRepo(structureRepo)
            , m_dataFlowRepo(dataFlowRepo)
            , m_constCondRepo(constCondRepo)
            , m_ircodeEventHandler(this)
        {
            m_structureRepo->getOrCreateStructure(dataFlowRepo->getGlobalStartNode());
        }

        std::shared_ptr<EventPipe> getEventPipe() {
            return m_ircodeEventHandler.getEventPipe();
        }

        void research(DataFlowNode* node) {
            DataFlowNode::PassSuccessors({ node }, std::function([this](DataFlowNode* node, bool& goNextNodes) {
                research(node, goNextNodes);
            }));
        }

    private:
        void research(DataFlowNode* node, bool& goNextNodes) {
            // stop if at least one of predecessors has no structure
            for (auto pred : node->predecessors) {
                if (!m_structureRepo->getLink(pred)) return;
            }
            auto block = node->variable->getSourceOperation()->getBlock();
            auto hash = m_structureRepo->getHash(node);
            if (node->type == DataFlowNode::Copy) {
                if (node->predecessors.size() == 1) {
                    auto pred = node->predecessors.front();
                    auto predLink = m_structureRepo->getLink(pred);
                    if (auto link = m_structureRepo->getLink(node)) {
                        if (link->structure != predLink->structure) {
                            m_structureRepo->removeStructure(link->structure);
                        }
                    }
                    m_structureRepo->addLink(node, predLink->structure, predLink->offset + node->offset);

                    if (node->offset == 0) {
                        auto structToConditions = findConditions(block);
                        auto it = structToConditions.find(predLink->structure);
                        if (it != structToConditions.end()) {
                            auto conditions = predLink->structure->conditions;
                            auto& newConditions = it->second;
                            conditions.merge(newConditions, true);
                            if (conditions.hash() != predLink->structure->conditions.hash()) {
                                auto structure = m_structureRepo->getOrCreateStructure(node);
                                structure->clearChilds();
                                predLink->structure->addChild(structure);
                                structure->conditions = conditions;
                            }
                        }
                    }
                } else {
                    auto structure = m_structureRepo->getOrCreateStructure(node);
                    structure->clearChilds();
                    structure->conditions.clear();
                    for (auto pred : node->predecessors) {
                        auto predLink = m_structureRepo->getLink(pred);
                        structure->addChild(predLink->structure);
                        structure->conditions.merge(predLink->structure->conditions);
                    }
                }
            }
            else if (node->type == DataFlowNode::Read) {
                assert(node->predecessors.size() == 1);
                auto structure = m_structureRepo->getOrCreateStructure(node);
                auto addrNode = node->predecessors.front();
                auto addrLink = m_structureRepo->getLink(addrNode);
                m_structureRepo->addField(addrLink->structure, addrLink->offset, structure);
            }
            else if (node->type == DataFlowNode::Write) {
                assert(node->predecessors.size() == 2);
                auto addrNode = node->predecessors.front();
                auto valueNode = node->predecessors.back();
                auto addrLink = m_structureRepo->getLink(addrNode);
                auto valueLink = m_structureRepo->getLink(valueNode);
                m_structureRepo->addField(addrLink->structure, addrLink->offset, valueLink->structure);
            }
            else if (node->type == DataFlowNode::Unknown) {
                m_structureRepo->getOrCreateStructure(node);
            }

            goNextNodes = hash != m_structureRepo->getHash(node);
        }

        std::map<Structure*, ConditionSet> findConditions(ircode::Block* block) {
            std::map<Structure*, ConditionSet> result;
            auto conditions = m_constCondRepo->findConditions(block);
            for (auto& [type, variable, value] : conditions) {
                if (type != ConstantCondition::EQUAL)
                    continue;
                if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(variable->getSourceOperation())) {
                    if (unaryOp->getId() == ircode::OperationId::LOAD) {
                        auto linearExpr = variable->getLinearExpr();
                        Offset offset = linearExpr.getConstTermValue();
                        for (auto& term : linearExpr.getTerms()) {
                            if (term.factor != 1 || term.value->getSize() != m_platform->getPointerSize())
                                continue;
                            if (auto ptrVar = std::dynamic_pointer_cast<ircode::Variable>(term.value)) {
                                if (auto ptrVarNode = m_dataFlowRepo->getNode(ptrVar)) {
                                    if (auto link = m_structureRepo->getLink(ptrVarNode)) {
                                        result[link->structure].insert(offset, value);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            return result;
        }
    };
};
