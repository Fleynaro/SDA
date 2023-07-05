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
    static const size_t StructureResearchTopic = TopicName("StructureResearchTopic");

    struct Structure;

    // When a structure is created
    struct StructureCreatedEvent : Event {
        Structure* structure;

        StructureCreatedEvent(Structure* structure)
            : Event(StructureResearchTopic)
            , structure(structure)
        {}
    };

    // When a child is added to a structure
    struct ChildAddedEvent : Event {
        Structure* structure;
        Structure* child;

        ChildAddedEvent(Structure* structure, Structure* child)
            : Event(StructureResearchTopic)
            , structure(structure)
            , child(child)
        {}
    };

    // When a child is removed from a structure
    struct ChildRemovedEvent : Event {
        Structure* structure;
        Structure* child;

        ChildRemovedEvent(Structure* structure, Structure* child)
            : Event(StructureResearchTopic)
            , structure(structure)
            , child(child)
        {}
    };

    // When a link is created
    struct LinkCreatedEvent : Event {
        DataFlowNode* node;
        Structure* structure;
        size_t offset;

        LinkCreatedEvent(DataFlowNode* node, Structure* structure, size_t offset = 0)
            : Event(StructureResearchTopic)
            , node(node)
            , structure(structure)
            , offset(offset)
        {}
    };

    class ConditionSet {
        std::map<size_t, std::set<size_t>> m_conditions;
    public:
        ConditionSet() = default;

        const std::map<size_t, std::set<size_t>>& values() const {
            return m_conditions;
        }

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
    };

    class StructureRepository
    {
        struct Link {
            Structure* structure;
            size_t offset;
            bool own;
        };
        std::list<Structure> m_structures;
        std::map<DataFlowNode*, Link> m_nodeToStructure;
        std::map<DataFlowNode*, size_t> m_nodeToConstant;
        std::shared_ptr<EventPipe> m_eventPipe;
    public:
        StructureRepository(std::shared_ptr<EventPipe> eventPipe);

        void setConstant(DataFlowNode* node, size_t value) {
            m_nodeToConstant[node] = value;
        }

        const size_t* getConstant(DataFlowNode* node) const {
            auto it = m_nodeToConstant.find(node);
            if (it == m_nodeToConstant.end()) {
                return 0;
            }
            return &it->second;
        }

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
            auto structure = &m_structures.back();
            m_eventPipe->send(StructureCreatedEvent(structure));
            return structure;
        }

        // <own> - create structure for <node> itself if it doesn't exist
        Structure* getOrCreateStructure(DataFlowNode* node, bool own = false) {
            auto link = getLink(node);
            if (link == nullptr || own && !link->own) {
                auto variable = node->getVariable();
                auto structure = createStructure(variable ? variable->getName(true) : "root");
                // add link
                m_nodeToStructure[node] = { structure, 0, true };
                m_eventPipe->send(LinkCreatedEvent(node, structure));
                return structure;
            }
            return link->structure;
        }

        void removeStructure(Structure* structure) {
            clearParents(structure);
            clearChilds(structure);
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
            addChild(fieldStructure, varStructure);
        }

        void addChild(Structure* structure, Structure* child) {
            if (std::find(structure->childs.begin(), structure->childs.end(), child) != structure->childs.end())
                return;
            structure->childs.push_back(child);
            child->parents.push_back(structure);
            m_eventPipe->send(ChildAddedEvent(structure, child));
        }

        void clearParents(Structure* structure) {
            for (auto parent : structure->parents) {
                parent->childs.remove(structure);
                m_eventPipe->send(ChildRemovedEvent(parent, structure));
            }
            structure->parents.clear();
        }

        void clearChilds(Structure* structure) {
            for (auto child : structure->childs) {
                child->parents.remove(structure);
                m_eventPipe->send(ChildRemovedEvent(structure, child));
            }
            structure->childs.clear();
        }

        void addLink(DataFlowNode* node, Structure* structure, size_t offset = 0) {
            m_nodeToStructure[node] = { structure, offset, false };
            m_eventPipe->send(LinkCreatedEvent(node, structure, offset));
        }

        void removeLink(DataFlowNode* node) {
            auto link = getLink(node);
            if (link) {
                if (link->own) {
                    removeStructure(link->structure);
                }
                m_nodeToStructure.erase(node);
            }
        }

        const Link* getLink(DataFlowNode* node) const {
            auto it = m_nodeToStructure.find(node);
            if (it == m_nodeToStructure.end()) {
                return nullptr;
            }
            return &it->second;
        }

        size_t getHash(DataFlowNode* node) const {
            if (node->type == DataFlowNode::Copy)
                return 0;
            auto link = getLink(node);
            if (!link) {
                if (auto constant = getConstant(node)) {
                    return std::hash<size_t>()(*constant);
                }
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

            void handleDataFlowNodeRemovedEvent(const DataFlowNodeRemovedEvent& event) {
                m_researcher->m_structureRepo->removeLink(event.node);
            }

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
                pipe->subscribeMethod(this, &EventHandler::handleDataFlowNodeRemovedEvent);
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
            DataFlowNode::PassSuccessors(node, std::function([this](DataFlowNode* node, bool& goNextNodes) {
                research(node, goNextNodes);
            }), m_program->getEventPipe());
        }

    private:
        void research(DataFlowNode* node, bool& goNextNodes) {
            auto hash = m_structureRepo->getHash(node);
            researchConstants(node);
            researchStructures(node);
            // go next if there are changes (for copy nodes go next always, see test StructureResearcherTest.Functions)
            goNextNodes = node->type == DataFlowNode::Copy || hash != m_structureRepo->getHash(node);
        }

        void researchConstants(DataFlowNode* node) {
            if (node->type == DataFlowNode::Copy) {
                if (node->predecessors.size() == 1) {
                    auto pred = node->predecessors.front();
                    if (auto predConstant = m_structureRepo->getConstant(pred)) {
                        m_structureRepo->setConstant(node, *predConstant);
                    }
                }
            }
            else if (node->type == DataFlowNode::Unknown) {
                if (auto constant = node->getConstant()) {
                    // see test StructureResearcherTest.If
                    m_structureRepo->setConstant(node, constant->getConstVarnode()->getValue());
                }
            }
        }

        void researchStructures(DataFlowNode* node) {
            if (node->type == DataFlowNode::Copy) {
                for (auto pred : node->predecessors) {
                    if (!m_structureRepo->getLink(pred)) return;
                }
                if (node->predecessors.size() == 1) {
                    auto pred = node->predecessors.front();
                    auto predLink = m_structureRepo->getLink(pred);
                    
                    bool isNewStructure = false;
                    if (node->offset == 0) {
                        // gather conditions (see test StructureResearcherTest.If)
                        auto variable = node->getVariable();
                        assert(variable);
                        auto block = variable->getSourceOperation()->getBlock();
                        auto structToConditions = findConditions(block);
                        auto it = structToConditions.find(predLink->structure);
                        if (it != structToConditions.end()) {
                            auto conditions = predLink->structure->conditions;
                            auto& newConditions = it->second;
                            conditions.merge(newConditions, true);
                            if (conditions.hash() != predLink->structure->conditions.hash()) {
                                auto structure = m_structureRepo->getOrCreateStructure(node, true);
                                m_structureRepo->clearParents(structure);
                                m_structureRepo->addChild(predLink->structure, structure);
                                structure->conditions = conditions;
                                isNewStructure = true;
                            }
                        }
                    }

                    if (!isNewStructure) {
                        if (auto link = m_structureRepo->getLink(node)) {
                            if (link->structure != predLink->structure) {
                                m_structureRepo->removeStructure(link->structure);
                            }
                        }
                        m_structureRepo->addLink(node, predLink->structure, predLink->offset + node->offset);
                    }
                } else {
                    // see test StructureResearcherTest::Functions
                    auto structure = m_structureRepo->getOrCreateStructure(node, true);
                    m_structureRepo->clearChilds(structure);
                    structure->conditions.clear();
                    for (auto pred : node->predecessors) {
                        auto predLink = m_structureRepo->getLink(pred);
                        m_structureRepo->addChild(structure, predLink->structure);
                        structure->conditions.merge(predLink->structure->conditions);
                    }
                    // since it is a copy node, we go next to gather all childs and conditions that were cleared here
                }
            }
            else if (node->type == DataFlowNode::Read) {
                assert(node->predecessors.size() == 1);
                auto addrNode = node->predecessors.front();
                auto addrLink = m_structureRepo->getLink(addrNode);
                if (!addrLink) return;
                auto structure = m_structureRepo->getOrCreateStructure(node, true);
                m_structureRepo->addField(addrLink->structure, addrLink->offset, structure);
            }
            else if (node->type == DataFlowNode::Write) {
                assert(node->predecessors.size() == 2);
                auto addrNode = node->predecessors.front();
                auto valueNode = node->predecessors.back();
                auto addrLink = m_structureRepo->getLink(addrNode);
                if (!addrLink) return;
                if (auto valueLink = m_structureRepo->getLink(valueNode)) {
                    m_structureRepo->addField(addrLink->structure, addrLink->offset, valueLink->structure);
                }
                else if (auto constValue = m_structureRepo->getConstant(valueNode)) {
                    // see test StructureResearcherTest.If
                    addrLink->structure->conditions.insert(addrLink->offset, *constValue);
                }
            }
            else if (node->type == DataFlowNode::Unknown) {
                if (node->getVariable()) {
                    m_structureRepo->getOrCreateStructure(node);
                }
            }      
        }

        std::map<Structure*, ConditionSet> findConditions(ircode::Block* block) {
            std::map<Structure*, ConditionSet> result;
            auto conditions = m_constCondRepo->findConditions(block);
            for (auto& [type, variable, value] : conditions) {
                if (type != ConstantCondition::EQUAL)
                    continue;
                if (auto loadOp = goToLoadOperation(variable->getSourceOperation())) {
                    auto linearExpr = ircode::Value::GetLinearExpr(loadOp->getInput(), true);
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
            return result;
        }

        const ircode::UnaryOperation* goToLoadOperation(const ircode::Operation* operation) {
            if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(operation)) {
                if (unaryOp->getId() == ircode::OperationId::COPY || unaryOp->getId() == ircode::OperationId::REF) {
                    if (auto var = std::dynamic_pointer_cast<ircode::Variable>(unaryOp->getInput())) {
                        return goToLoadOperation(var->getSourceOperation());
                    }
                }
                else if (unaryOp->getId() == ircode::OperationId::LOAD) {
                    return unaryOp;
                }
            }
            return nullptr;
        }
    };
};
