#pragma once
#include "StructureResearcher.h"
#include "SDA/Core/Event/EventBatch.h"

namespace sda::researcher
{
    struct FieldStructureGroup {
        std::set<Structure*> structures;
        std::set<Structure*> inputs;
        std::set<Structure*> outputs;
        std::set<Structure*> parents;
        std::set<Structure*> childs;

        void add(Structure* structure) {
            structures.insert(structure);
        }

        void remove(Structure* structure) {
            structures.erase(structure);
        }

        void finalize() {
            inputs.clear();
            outputs.clear();
            for (auto structure : structures) {
                inputs.insert(structure->inputs.begin(), structure->inputs.end());
                outputs.insert(structure->outputs.begin(), structure->outputs.end());
            }
            parents.clear();
            childs.clear();
            for (auto structure : structures) {
                parents.insert(structure->parents.begin(), structure->parents.end());
                childs.insert(structure->childs.begin(), structure->childs.end());
            }
        }

        Structure* getSomeStructure() const {
            return *structures.begin();
        }
    };

    class ClassFieldRangeSet {
        using LabelType = size_t;
        struct Range {
            std::set<LabelType> labels;
            size_t maxOffset = 0;

            bool intersects(const Range& range) {
                for (auto label : range.labels) {
                    if (labels.find(label) != labels.end())
                        return true;
                }
                return false;
            }

            bool includes(const Range& range) {
                for (auto label : range.labels) {
                    if (labels.find(label) == labels.end())
                        return false;
                }
                return true;
            }

            void merge(const Range& range) {
                for (auto label : range.labels) {
                    labels.insert(label);
                }
                //maxOffset = std::max(maxOffset, range.maxOffset);
            }
        };
        std::list<Range> m_ranges;
    public:
        using Ranges = std::list<Range>;

        ClassFieldRangeSet() = default;

        const std::list<Range>& getAllRanges() const {
            return m_ranges;
        }

        std::list<Range> getRangesContaining(size_t offset) const {
            std::list<Range> ranges;
            for (auto it = m_ranges.begin(); it != m_ranges.end(); ++it) {
                if (offset <= it->maxOffset) {
                    bool isAlreadyAdded = false;
                    for (auto& range : ranges) {
                        if (range.intersects(*it)) {
                            isAlreadyAdded = true;
                            break;
                        }
                    }
                    if (!isAlreadyAdded) {
                        ranges.push_back(*it);
                    }
                }
            }
            return ranges;
        }

        void addRange(const std::set<LabelType>& labels, size_t maxOffset) {
            auto newRange = Range { labels, maxOffset };
            for (auto& range : m_ranges) {
                if (range.intersects(newRange)) {
                    if (range.maxOffset <= newRange.maxOffset) {
                        range.merge(newRange);
                    } else {
                        newRange.merge(range);
                    }
                }
            }
            m_ranges.emplace_front(newRange);
        }

        void finalize() {
            // sort
            m_ranges.sort([](const auto& a, const auto& b) {
                if (a.labels.size() == b.labels.size())
                    return a.maxOffset > b.maxOffset;
                return a.labels.size() > b.labels.size();
            });

            // remove included ranges
            auto it1 = m_ranges.begin();
            while (it1 != m_ranges.end()) {
                auto it2 = std::next(it1);
                while (it2 != m_ranges.end()) {
                    if (it1->maxOffset >= it2->maxOffset && it1->includes(*it2)) {
                        it2 = m_ranges.erase(it2);
                    } else {
                        it2++;
                    }
                }
                it1++;
            }
        }
    };

    class ClassRepository
    {
        struct StructureInfo {
            Structure* structure;
            ConstantSet conditions;
            ConstantSet constants;
            FieldStructureGroup* group = nullptr;

            ConstantSet getLabelSet() const {
                ConstantSet set;
                set.merge(conditions);
                set.merge(constants);
                return set;
            }

            const std::set<Structure*>& getInputs() {
                return group ? group->inputs : structure->inputs;
            }

            const std::set<Structure*>& getOutputs() {
                return group ? group->outputs : structure->outputs;
            }

            const std::set<Structure*>& getParents() {
                return group ? group->parents : structure->parents;
            }

            const std::set<Structure*>& getChilds() {
                return group ? group->childs : structure->childs;
            }
        };
        std::map<Structure*, StructureInfo> m_structureToInfo;
        std::list<FieldStructureGroup> m_fieldStructureGroups;
        std::shared_ptr<EventPipe> m_eventPipe;
    public:
        ClassRepository(std::shared_ptr<EventPipe> eventPipe)
            : m_eventPipe(eventPipe)
        {}

        const std::list<FieldStructureGroup>& getAllFieldStructureGroups() const {
            return m_fieldStructureGroups;
        }

        StructureInfo* getStructureInfo(Structure* structure) {
            auto it = m_structureToInfo.find(structure);
            if (it == m_structureToInfo.end()) {
                it = m_structureToInfo.emplace(structure, StructureInfo { structure }).first;
            }
            return &it->second;
        }

        void removeStructure(Structure* structure) {
            m_structureToInfo.erase(structure);
        }

        void addFieldStructureGroup(const FieldStructureGroup& group, std::set<FieldStructureGroup*>& changedGroups) {
            auto info = getStructureInfo(group.getSomeStructure());
            if (info->group && group.structures == info->group->structures)
                return;
            m_fieldStructureGroups.push_back(group);
            auto newGroup = &m_fieldStructureGroups.back();
            changedGroups.insert(newGroup);
            for (auto structure : newGroup->structures) {
                auto info = getStructureInfo(structure);
                if (info->group) {
                    info->group->remove(structure);
                    if (info->group->structures.empty()) {
                        // if group is empty, remove it
                        m_fieldStructureGroups.remove_if([&](const auto& group) {
                            return group.structures.empty();
                        });
                    } else {
                        changedGroups.insert(info->group);
                    }
                }
                info->group = newGroup;
            }
            for (auto group : changedGroups)
                group->finalize();
        }

        void gatherStructuresInGroup(Structure* structure, std::set<Structure*>& result) {
            if (result.find(structure) != result.end())
                return;
            result.insert(structure);
            auto info = getStructureInfo(structure);
            for (auto output : info->getOutputs()) {
                gatherStructuresInGroup(output, result);
            }
            for (auto input : info->getInputs()) {
                gatherStructuresInGroup(input, result);
            }
        }
    };

    class ClassResearcher
    {
        ircode::Program* m_program;
        Platform* m_platform;
        ClassRepository* m_classRepo;
        StructureRepository* m_structureRepo;

        class EventHandler
        {
            ClassResearcher* m_researcher;

            void handleStructureRemovedEvent(const StructureRemovedEvent& event) {
                m_researcher->m_classRepo->removeStructure(event.structure);
            }

            void handleStructureUpdatedEventBatch(const EventBatch<StructureUpdatedEvent>& event) {
                std::list<Structure*> startStructures;
                for (auto& e : event.events) {
                    startStructures.push_back(e.structure);
                }
                m_researcher->research(startStructures);
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
                };
                auto data = std::make_shared<Data>();
                auto filter = EventPipe::FilterTopic(StructureResearchTopic);
                auto commitEmitter = std::function([data](const EventNext& next) {
                    EventBatch<StructureUpdatedEvent> batch(StructureResearchTopic);
                    for (auto structure : data->updatedStructures) {
                        batch.events.push_back(StructureUpdatedEvent(structure));
                    }
                    data->updatedStructures.clear();
                    next(batch);
                });
                std::shared_ptr<EventPipe> commitPipeIn;
                auto pipe = OptimizedCommitPipe(filter, commitPipeIn, commitEmitter);
                commitPipeIn->subscribe(std::function([data](const StructureCreatedEvent& event) {
                    // creating event is treated as updating event
                    data->add(event.structure);
                }));
                commitPipeIn->subscribe(std::function([data](const StructureRemovedEvent& event) {
                    data->updatedStructures.remove(event.structure);
                }));
                commitPipeIn->subscribe(std::function([data](const ChildAddedEvent& event) {
                    data->add(event.structure);
                    data->add(event.child);
                }));
                commitPipeIn->subscribe(std::function([data](const ChildRemovedEvent& event) {
                    data->add(event.structure);
                    data->add(event.child);
                }));
                return pipe;
            }
        public:
            EventHandler(ClassResearcher* researcher) : m_researcher(researcher) {}

            std::shared_ptr<EventPipe> getEventPipe() {
                auto pipe = EventPipe::New();
                pipe
                    ->connect(GetOptimizedEventPipe())
                    ->subscribeMethod(this, &EventHandler::handleStructureUpdatedEventBatch);
                pipe->subscribeMethod(this, &EventHandler::handleStructureRemovedEvent);
                return pipe;
            }
        };
        EventHandler m_ircodeEventHandler;
    public:
        ClassResearcher(
            ircode::Program* program,
            Platform* platform,
            ClassRepository* classRepo,
            StructureRepository* structureRepo
        )
            : m_program(program)
            , m_platform(platform)
            , m_classRepo(classRepo)
            , m_structureRepo(structureRepo)
            , m_ircodeEventHandler(this)
        {
        }

        std::shared_ptr<EventPipe> getEventPipe() {
            return m_ircodeEventHandler.getEventPipe();
        }

        void research(std::list<Structure*> startStructures) {
            auto structuresToProcess = startStructures;
            while (!structuresToProcess.empty()) {
                auto structureToProcess = structuresToProcess.front();
                // get all structures in the group
                std::set<Structure*> structuresInGroup;
                m_classRepo->gatherStructuresInGroup(structureToProcess, structuresInGroup);
                for (auto structure : structuresInGroup) {
                    auto it = std::find(structuresToProcess.begin(), structuresToProcess.end(), structure);
                    if (it != structuresToProcess.end()) {
                        // propagate conditions & constants
                        structure->passDescendants(std::function([this](Structure* structure, const std::function<void(Structure* structure)>& next) {
                            propagateConditions(structure, next);
                        }));
                        structure->passDescendants(std::function([this](Structure* structure, const std::function<void(Structure* structure)>& next) {
                            propagateConstants(structure, next);
                        }));
                        // remove
                        structuresToProcess.erase(it);
                    }
                }

                ClassFieldRangeSet classFieldRangeSet;
                std::map<size_t, std::map<size_t, std::list<Structure*>>> structuresByFieldAndLabel;
                for (auto structure : structuresInGroup) {
                    auto info = m_classRepo->getStructureInfo(structure);
                    auto labels = getLabels(info->getLabelSet(), structure);

                    // fill labelsToMaxOffset
                    size_t lastOffset = -1;
                    if (!structure->fields.empty()) {
                        lastOffset = structure->fields.rbegin()->first;
                    }
                    if (!structure->conditions.values().empty()) {
                        lastOffset = std::max(lastOffset, structure->conditions.values().rbegin()->first);
                    }
                    if (lastOffset != -1) {
                        classFieldRangeSet.addRange(labels, lastOffset);
                    }

                    // fill structuresByFieldAndLabel (indexing structures by [field, label])
                    for (auto& [offset, field] : structure->fields) {
                        for (auto label : labels) {
                            structuresByFieldAndLabel[offset][label].push_back(structure);
                        }
                    }
                }
                classFieldRangeSet.finalize();

                for (auto& [offset, structuresWithField] : structuresByFieldAndLabel) {
                    auto ranges = classFieldRangeSet.getRangesContaining(offset);
                    for (auto range : ranges) {
                        FieldStructureGroup newGroup;
                        for (auto label : range.labels) {
                            for (auto structure : structuresWithField[label]) {
                                newGroup.add(structure->fields[offset]);
                            }
                        }
                        std::set<FieldStructureGroup*> changedGroups;
                        m_classRepo->addFieldStructureGroup(newGroup, changedGroups);
                        for (auto group : changedGroups) {
                            structuresToProcess.push_back(group->getSomeStructure());
                        }
                    }
                }
            }
        }

    private:
        void propagateConditions(Structure* structure, const std::function<void(Structure* structure)>& next) {
            auto info = m_classRepo->getStructureInfo(structure);

            // create new conditions by merging inputs and current conditions
            ConstantSet newConditions;
            for (auto input : info->getInputs()) {
                auto inputInfo = m_classRepo->getStructureInfo(input);
                newConditions.merge(inputInfo->conditions);
            }
            for (auto child : info->getChilds()) {
                auto childInfo = m_classRepo->getStructureInfo(child);
                newConditions.merge(childInfo->conditions);
            }
            newConditions.merge(structure->conditions, true);
            auto newConditionsHash = newConditions.hash();

            bool goNext = false;
            if (info->group) {
                for (auto groupStruct : info->group->structures) {
                    auto groupStructInfo = m_classRepo->getStructureInfo(groupStruct);
                    goNext = goNext || groupStruct->conditions.hash() != newConditionsHash;
                    groupStructInfo->conditions = newConditions;
                }
            } else {
                goNext = goNext || info->conditions.hash() != newConditionsHash;
                info->conditions = newConditions;
            }

            if (goNext) {
                for (auto output : info->getOutputs())
                    next(output);
                for (auto parent : info->getParents())
                    next(parent);
            }
        }

        void propagateConstants(Structure* structure, const std::function<void(Structure* structure)>& next) {
            auto info = m_classRepo->getStructureInfo(structure);

            // create new constants by merging inputs and current constants
            ConstantSet newConstants;
            for (auto input : info->getInputs()) {
                auto inputInfo = m_classRepo->getStructureInfo(input);
                newConstants.merge(inputInfo->constants);
            }
            newConstants.merge(structure->conditions, true);
            newConstants.merge(structure->constants, true);
            auto newConstantsHash = newConstants.hash();

            bool goNext = false;
            if (info->group) {
                for (auto groupStruct : info->group->structures) {
                    auto groupStructInfo = m_classRepo->getStructureInfo(groupStruct);
                    goNext = goNext || groupStruct->constants.hash() != newConstantsHash;
                    groupStructInfo->constants = newConstants;
                }
            } else {
                goNext = goNext || info->constants.hash() != newConstantsHash;
                info->constants = newConstants;
            }

            if (goNext) {
                for (auto output : info->getOutputs())
                    next(output);
            }
        }

        std::set<size_t> getLabels(const ConstantSet& set, Structure* structure) {
            std::set<size_t> labels;
            auto it = set.values().find(0x0); // label (type) field at offset 0x0
            if (it != set.values().end()) {
                labels = it->second;
            } else {
                labels.insert(-1);
            }
            return labels;
        }
    };
};
