#pragma once
#include "DataFlowResearcher.h"
#include "ConstConditionResearcher.h"
#include "SDA/Core/Commit.h"
#include "SDA/Core/DataType/StructureDataType.h"
#include "SDA/Core/IRcode/IRcodeBlock.h"
#include "SDA/Core/IRcode/IRcodeEvents.h"
#include "SDA/Core/IRcode/IRcodeHelper.h"
#include "SDA/Core/Utils/String.h"
#include "SDA/Core/Event/EventBatch.h"

namespace sda::researcher
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

    // When a structure is updated
    struct StructureUpdatedEvent : Event {
        Structure* structure;

        StructureUpdatedEvent(Structure* structure)
            : Event(StructureResearchTopic)
            , structure(structure)
        {}
    };

    // When a structure is removed
    struct StructureRemovedEvent : Event {
        Structure* structure;

        StructureRemovedEvent(Structure* structure)
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

    class ConstantSet {
        std::map<size_t, std::set<size_t>> m_conditions;
    public:
        ConstantSet() = default;

        const std::map<size_t, std::set<size_t>>& values() const {
            return m_conditions;
        }

        void insert(size_t offset, size_t value) {
            m_conditions[offset].insert(value);
        }

        void remove(size_t offset) {
            auto it = m_conditions.find(offset);
            if (it != m_conditions.end()) {
                m_conditions.erase(it);
            }
        }

        void merge(const ConstantSet& other, bool clear = false) {
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
        size_t id;
        std::string name;
        size_t version = 0;
        DataFlowNode* sourceNode = nullptr;
        std::set<DataFlowNode*> linkedNodes;
        std::set<Structure*> parents;
        std::set<Structure*> childs;
        std::set<Structure*> inputs;
        std::set<Structure*> outputs;
        std::map<size_t, Structure*> fields;
        ConstantSet conditions;
        ConstantSet constants;

        const std::string& getName() const {
            return name;
        }

        DataFlowNode* getSourceNode() const {
            return sourceNode;
        }

        const std::set<DataFlowNode*>& getLinkedNodes() const {
            return linkedNodes;
        }

        const std::set<Structure*>& getParents() const {
            return parents;
        }

        const std::set<Structure*>& getChilds() const {
            return childs;
        }

        const std::set<Structure*>& getInputs() const {
            return inputs;
        }

        const std::set<Structure*>& getOutputs() const {
            return outputs;
        }

        const std::map<size_t, Structure*>& getFields() const {
            return fields;
        }

        const ConstantSet& getConditions() const {
            return conditions;
        }

        const ConstantSet& getConstants() const {
            return constants;
        }

        void passDescendants(std::function<void(Structure* structure, const std::function<void(Structure* structure)>& next)> callback);
    };

    class StructureRepository
    {
        struct Link {
            Structure* structure;
            size_t offset;
            size_t version;
            bool own; // the node has its own structure
        };
        std::list<Structure> m_structures;
        std::map<DataFlowNode*, Link> m_nodeToStructure;
        std::map<size_t, Structure*> m_nameToStructure;
        std::map<DataFlowNode*, size_t> m_nodeToConstant;
        std::map<DataFlowNode*, size_t> m_nodeToHash;
        size_t m_idCounter = 0;
        std::shared_ptr<EventPipe> m_eventPipe;
    public:
        StructureRepository(std::shared_ptr<EventPipe> eventPipe);

        Structure* getStructureByName(const std::string& name) {
            auto hash = std::hash<std::string>()(name);
            auto it = m_nameToStructure.find(hash);
            if (it == m_nameToStructure.end()) {
                return nullptr;
            }
            return it->second;
        }

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

        std::list<Structure*> getAllStructures() {
            std::list<Structure*> result;
            for (auto& structure : m_structures) {
                result.push_back(&structure);
            }
            return result;
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
            m_structures.emplace_back(Structure { m_idCounter++, name });
            auto structure = &m_structures.back();
            m_nameToStructure[std::hash<std::string>()(structure->name)] = structure;
            m_eventPipe->send(StructureCreatedEvent(structure));
            return structure;
        }

        Structure* createStructure(DataFlowNode* node) {
            auto variable = node->getVariable();
            auto structure = createStructure(variable ? variable->getName(true) : "root");
            structure->sourceNode = node;
            addLink(node, structure, 0, true);
            return structure;
        }

        Structure* getOrCreateStructure(DataFlowNode* node) {
            auto link = getLink(node);
            if (link == nullptr || !link->own) {
                return createStructure(node);
            }
            return link->structure;
        }

        Structure* getStructure(DataFlowNode* node) {
            auto link = getLink(node);
            if (link == nullptr) {
                return nullptr;
            }
            return link->structure;
        }

        void removeStructure(Structure* structure) {
            clearStructure(structure);
            m_eventPipe->send(StructureRemovedEvent(structure));
            // remove links
            for (auto node : structure->linkedNodes) {
                m_nodeToStructure.erase(node);
            }
            // remove structure
            m_nameToStructure.erase(std::hash<std::string>()(structure->name));
            m_structures.remove_if([structure](const Structure& s) {
                return &s == structure;
            });
        }

        void clearStructure(Structure* structure) {
            // clear parents
            for (auto parent : structure->parents) {
                parent->childs.erase(structure);
                m_eventPipe->send(ChildRemovedEvent(parent, structure));
            }
            structure->parents.clear();

            // clear childs
            for (auto child : structure->childs) {
                child->parents.erase(structure);
                m_eventPipe->send(ChildRemovedEvent(structure, child));
            }
            structure->childs.clear();

            // clear inputs
            for (auto input : structure->inputs) {
                input->outputs.erase(structure);
            }
            structure->inputs.clear();

            // clear outputs
            for (auto output : structure->outputs) {
                output->inputs.erase(structure);
            }
            structure->outputs.clear();

            // remove fields
            for (auto& [offset, field] : structure->fields) {
                removeStructure(field);
            }
            structure->fields.clear();
            
            // remove conditions
            structure->conditions.clear();
        }

        void markAsUpdated(Structure* structure) {
            structure->version++;
            addLink(structure->sourceNode, structure, 0, true);
        }

        void addField(Structure* structure, size_t offset, Structure* varStructure, bool write) {
            auto it = structure->fields.find(offset);
            if (it == structure->fields.end()) {
                auto fieldStructure = createStructure(structure->name + "_0x" + utils::ToHex(offset));
                it = structure->fields.emplace(offset, fieldStructure).first;
            }
            auto fieldStructure = it->second;
            addChild(fieldStructure, varStructure);
            if (write) {
                // if writing to the field
                addOutput(varStructure, fieldStructure);
            } else {
                // if reading from the field
                addOutput(fieldStructure, varStructure);
            }
        }

        void addChild(Structure* structure, Structure* child) {
            if (structure->childs.find(child) != structure->childs.end())
                return;
            structure->childs.insert(child);
            child->parents.insert(structure);
            m_eventPipe->send(ChildAddedEvent(structure, child));
        }

        void addOutput(Structure* structure, Structure* output) {
            if (structure->outputs.find(output) != structure->outputs.end())
                return;
            structure->outputs.insert(output);
            output->inputs.insert(structure);
        }

        // <own> - create structure for <node> itself if it doesn't exist
        void addLink(DataFlowNode* node, Structure* structure, size_t offset = 0, bool own = false) {
            structure->linkedNodes.insert(node);
            m_nodeToStructure[node] = { structure, offset, structure->version, own };
            m_eventPipe->send(LinkCreatedEvent(node, structure, offset));
        }

        const Link* getLink(DataFlowNode* node) const {
            auto it = m_nodeToStructure.find(node);
            if (it == m_nodeToStructure.end()) {
                return nullptr;
            }
            return &it->second;
        }

        void removeNode(DataFlowNode* node) {
            auto link = getLink(node);
            if (link) {
                link->structure->linkedNodes.erase(node);
                if (link->own) {
                    removeStructure(link->structure);
                }
                m_nodeToStructure.erase(node);
            }
            if (auto it = m_nodeToConstant.find(node); it != m_nodeToConstant.end()) {
                m_nodeToConstant.erase(it);
            }
            if (auto it = m_nodeToHash.find(node); it != m_nodeToHash.end()) {
                m_nodeToHash.erase(it);
            }
        }
        
        size_t* getHashPtr(DataFlowNode* node) {
            auto it = m_nodeToHash.find(node);
            if (it == m_nodeToHash.end()) {
                it = m_nodeToHash.emplace(node, 0).first;
            }
            return &it->second;
        }

        size_t getHash(DataFlowNode* node) const {
            auto link = getLink(node);
            if (!link) {
                if (auto constant = getConstant(node)) {
                    return std::hash<size_t>()(*constant);
                }
                return 0;
            }
            size_t result = 0;
            boost::hash_combine(result, link->structure->id);
            boost::hash_combine(result, link->version);
            return result;
        }

        static std::shared_ptr<EventPipe> GetOptimizedEventPipe() {
            struct Data {
                std::list<Structure*> updatedStructures;

                void add(Structure* structure) {
                    auto it = std::find(updatedStructures.begin(), updatedStructures.end(), structure);
                    if (it == updatedStructures.end()) {
                        updatedStructures.push_back(structure);
                    }
                }

                void remove(Structure* structure) {
                    updatedStructures.remove(structure);
                }
            };
            auto data = std::make_shared<Data>();
            auto commitEndHandler = std::function([data](const EventNext& next) {
                if (data->updatedStructures.empty()) return;
                EventBatch<StructureUpdatedEvent> batch(StructureResearchTopic);
                for (auto structure : data->updatedStructures) {
                    batch.events.push_back(StructureUpdatedEvent(structure));
                }
                data->updatedStructures.clear();
                next(batch);
            });
            auto [optPipe, commitPipeIn, _] = OptimizedCommitPipe(commitEndHandler);
            commitPipeIn->subscribe(std::function([data](const StructureCreatedEvent& event) {
                // creating event is treated as updating event
                data->add(event.structure);
            }));
            commitPipeIn->subscribe(std::function([data](const StructureRemovedEvent& event) {
                data->remove(event.structure);
            }));
            commitPipeIn->subscribe(std::function([data](const ChildAddedEvent& event) {
                data->add(event.structure);
                data->add(event.child);
            }));
            commitPipeIn->subscribe(std::function([data](const ChildRemovedEvent& event) {
                data->add(event.structure);
                data->add(event.child);
            }));
            commitPipeIn->subscribe(std::function([data](const LinkCreatedEvent& event) {
                data->add(event.structure);
            }));
            return optPipe;
        }
    };

    class StructureResearcher
    {
        ircode::Program* m_program;
        StructureRepository* m_structureRepo;
        DataFlowRepository* m_dataFlowRepo;
        researcher::ConstConditionRepository* m_constCondRepo;

        class EventHandler
        {
            StructureResearcher* m_researcher;

            void handleDataFlowNodeRemovedEvent(const DataFlowNodeRemovedEvent& event) {
                m_researcher->m_structureRepo->removeNode(event.node);
            }

            void handleDataFlowNodeUpdatedEvent(const DataFlowNodeUpdatedEvent& event) {
                m_researcher->research(event.node);
            }

            static std::shared_ptr<EventPipe> GetOptimizedEventPipe() {
                struct Data {
                    std::list<DataFlowNode*> updatedNodes;
                };
                auto data = std::make_shared<Data>();
                auto commitEndHandler = std::function([data](const EventNext& next) {
                    while (!data->updatedNodes.empty()) {
                        auto node = data->updatedNodes.front();
                        data->updatedNodes.pop_front();
                        next(DataFlowNodeUpdatedEvent(node));
                    }
                });
                auto [optPipe, commitPipeIn, _] = OptimizedCommitPipe(commitEndHandler);
                commitPipeIn->subscribe(std::function([data](const DataFlowNodeCreatedEvent& event) {
                    // creating event is treated as updating event
                    data->updatedNodes.push_back(event.node);
                }));
                commitPipeIn->subscribe(std::function([data](const DataFlowNodeRemovedEvent& event) {
                    data->updatedNodes.remove(event.node);
                }));
                commitPipeIn->subscribe(std::function([data](const DataFlowNodeUpdatedEvent& event) {
                    auto it = std::find(data->updatedNodes.begin(), data->updatedNodes.end(), event.node);
                    if (it == data->updatedNodes.end()) {
                        data->updatedNodes.push_back(event.node);
                    }
                }));
                return optPipe;
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
            StructureRepository* structureRepo,
            DataFlowRepository* dataFlowRepo,
            researcher::ConstConditionRepository* constCondRepo
        )
            : m_program(program)
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

        void research(DataFlowNode* node);

    private:
        void research(DataFlowNode* node, bool& goNextNodes, const std::function<void(DataFlowNode* node)>& next);

        void researchConstants(DataFlowNode* node);

        void researchStructures(DataFlowNode* node, const std::function<void(DataFlowNode* node)>& next);

        std::map<Structure*, ConstantSet> findConditions(ircode::Block* block);

        const ircode::UnaryOperation* goToLoadOperation(const ircode::Operation* operation);
    };

    static std::list<researcher::Structure*> SortStructuresByName(std::list<researcher::Structure*> structures) {
        structures.sort([](researcher::Structure* a, researcher::Structure* b) {
            return a->name < b->name;
        });
        return structures;
    }

    static std::list<researcher::Structure*> SortStructuresByName(const std::set<researcher::Structure*>& structures) {
        std::list<researcher::Structure*> result;
        for (auto structure : structures) {
            result.push_back(structure);
        }
        return SortStructuresByName(result);
    }

    static void GatherAllChildStructures(researcher::Structure* rootStructure, std::list<researcher::Structure*>& result) {
        std::list<researcher::Structure*> structuresToVisit;
        structuresToVisit.push_back(rootStructure);
        while (!structuresToVisit.empty()) {
            auto structure = structuresToVisit.front();
            structuresToVisit.pop_front();
            result.push_back(structure);
            for (auto child : SortStructuresByName(structure->childs)) {
                if (std::find(result.begin(), result.end(), child) != result.end()) {
                    continue;
                }
                structuresToVisit.push_back(child);
            }
        }
    }

    static void PrintStructure(std::stringstream& ss, researcher::Structure* structure) {
        ss << "struct " << structure->name << " ";
        if (!structure->parents.empty()) {
            ss << ": ";
            auto parents = SortStructuresByName(structure->parents);
            for (auto parent : parents) {
                ss << parent->name;
                if (parent != parents.back()) {
                    ss << ", ";
                }
            }
            ss << " ";
        }
        ss << "{" << std::endl;
        std::set<size_t> fieldOffsets;
        for (auto& [offset, _] : structure->fields) {
            fieldOffsets.insert(offset);
        }
        researcher::ConstantSet labelSet;
        labelSet.merge(structure->conditions);
        labelSet.merge(structure->constants, true);
        for (auto& [offset, _] : labelSet.values()) {
            fieldOffsets.insert(offset);
        }
        for (auto offset : fieldOffsets) {
            std::string sep;
            std::stringstream fieldValues;
            // structures
            auto it = structure->fields.find(offset);
            if (it != structure->fields.end()) {      
                auto childs = SortStructuresByName(it->second->childs);
                for (auto child : childs) {
                    fieldValues << sep << child->name;
                    sep = ", ";
                }
            }
            // labels
            auto it2 = labelSet.values().find(offset);
            if (it2 != labelSet.values().end()) {
                for (auto value : it2->second) {
                    fieldValues << sep << "0x" << utils::ToHex(value);
                    sep = ", ";
                }
            }
            ss << "    " << "0x" << utils::ToHex(offset) << ": " << fieldValues.str() << std::endl;
        }
        ss << "}";
    }

    static std::string PrintStructureStr(researcher::Structure* structure) {
        std::stringstream ss;
        PrintStructure(ss, structure);
        return ss.str();
    }
};
