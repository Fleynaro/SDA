#pragma once
#include "StructureResearcher.h"

namespace sda::researcher
{
    class FieldStructureGroup {
        std::set<Structure*> m_structures;
    public:
        FieldStructureGroup() = default;

        void add(Structure* structure) {
            m_structures.insert(structure);
        }

        void remove(Structure* structure) {
            m_structures.erase(structure);
        }

        bool empty() const {
            return m_structures.empty();
        }

        std::set<Structure*> getInputs() {
            // TODO: optimize? (cache inputs)
            std::set<Structure*> inputs;
            for (auto structure : m_structures) {
                inputs.insert(structure->inputs.begin(), structure->inputs.end());
            }
            return inputs;
        }

        std::set<Structure*> getOutputs() {
            std::set<Structure*> outputs;
            for (auto structure : m_structures) {
                outputs.insert(structure->outputs.begin(), structure->outputs.end());
            }
            return outputs;
        }

        std::set<Structure*> getParents() {
            std::set<Structure*> parents;
            for (auto structure : m_structures) {
                parents.insert(structure->parents.begin(), structure->parents.end());
            }
            return parents;
        }

        std::set<Structure*> getChilds() {
            std::set<Structure*> childs;
            for (auto structure : m_structures) {
                childs.insert(structure->childs.begin(), structure->childs.end());
            }
            return childs;
        }

        const std::set<Structure*>& getStructures() const {
            return m_structures;
        }

        Structure* getSomeStructure() const {
            return *m_structures.begin();
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

    static const size_t ClassResearchTopic = TopicName("ClassResearchTopic");

    // When class label info is added
    struct ClassLabelInfoAddedEvent : Event {
        Structure* structure;

        ClassLabelInfoAddedEvent(Structure* structure)
            : Event(ClassResearchTopic)
            , structure(structure)
        {}
    };

    struct ClassLabelInfo {
        pcode::InstructionOffset structureInstrOffset;
        pcode::InstructionOffset sourceInstrOffset;
        size_t labelOffset;
    };

    struct StructureInfo {
        Structure* structure;
        ConstantSet conditions;
        ConstantSet constants;
        size_t labelOffset = size_t(-1);
        std::set<size_t> labels;
        FieldStructureGroup* group = nullptr;

        size_t getLabelOffset() const {
            return labelOffset;
        }

        const std::set<size_t>& getLabels() {
            return labels;
        }

        ConstantSet getLabelSet() const {
            ConstantSet set;
            set.merge(conditions);
            set.merge(constants);
            return set;
        }

        std::set<Structure*> getInputs() {
            return group ? group->getInputs() : structure->inputs;
        }

        std::set<Structure*> getOutputs() {
            return group ? group->getOutputs() : structure->outputs;
        }

        std::set<Structure*> getParents() {
            return group ? group->getParents() : structure->parents;
        }

        std::set<Structure*> getChilds() {
            return group ? group->getChilds() : structure->childs;
        }

        FieldStructureGroup* getGroup() {
            return group;
        }
    };

    class ClassRepository
    {
        std::map<Structure*, StructureInfo> m_structureToInfo;
        std::map<sda::Offset, ClassLabelInfo> m_classLabelInfos;
        std::list<FieldStructureGroup> m_fieldStructureGroups;
        std::shared_ptr<EventPipe> m_eventPipe;
    public:
        ClassRepository(std::shared_ptr<EventPipe> eventPipe)
            : m_eventPipe(eventPipe)
        {}

        void addUserDefinedLabelOffset(Structure* structure, const ClassLabelInfo& info) {
            m_classLabelInfos.emplace(info.structureInstrOffset, info);
            m_eventPipe->send(ClassLabelInfoAddedEvent(structure));
        }

        size_t getUserDefinedLabelOffset(const std::set<Structure*>& structures) const {
            size_t labelOffset = -1;
            for (auto structure : structures) {
                for (auto linkNode : structure->linkedNodes) {
                    if (auto var = linkNode->getVariable()) {
                        if (auto instr = var->getSourceOperation()->getPcodeInstruction()) {
                            auto it = m_classLabelInfos.find(instr->getOffset());
                            if (it != m_classLabelInfos.end()) {
                                labelOffset = it->second.labelOffset;
                                break;
                            }
                        }
                    }
                }
                if (labelOffset != -1) break;
            }
            return labelOffset;
        }

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
            auto info = getStructureInfo(structure);
            if (info->group) {
                removeStructureFromGroup(info);
            }
            m_structureToInfo.erase(structure);
        }

        void addFieldStructureGroup(const FieldStructureGroup& group, std::set<FieldStructureGroup*>& changedGroups) {
            auto info = getStructureInfo(group.getSomeStructure());
            if (info->group && group.getStructures() == info->group->getStructures())
                return;
            m_fieldStructureGroups.push_back(group);
            auto newGroup = &m_fieldStructureGroups.back();
            changedGroups.insert(newGroup);
            for (auto structure : newGroup->getStructures()) {
                auto info = getStructureInfo(structure);
                if (info->group) {
                    if (auto changedGroup = removeStructureFromGroup(info)) {
                        changedGroups.insert(changedGroup);
                    }
                }
                info->group = newGroup;
            }
        }

        FieldStructureGroup* removeStructureFromGroup(StructureInfo* info) {
            info->group->remove(info->structure);
            if (!info->group->empty()) {
                return info->group;
            }
            // if group is empty, remove it
            m_fieldStructureGroups.remove_if([&](const auto& group) {
                return group.empty();
            });
            return nullptr;
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

            void handleClassLabelInfoAddedEvent(const ClassLabelInfoAddedEvent& event) {
                m_researcher->research({ event.structure });
            }
        public:
            EventHandler(ClassResearcher* researcher) : m_researcher(researcher) {}

            std::shared_ptr<EventPipe> getEventPipe() {
                auto pipe = EventPipe::New();
                pipe
                    ->connect(StructureRepository::GetOptimizedEventPipe())
                    ->subscribeMethod(this, &EventHandler::handleStructureUpdatedEventBatch);
                pipe->subscribeMethod(this, &EventHandler::handleClassLabelInfoAddedEvent);
                pipe->subscribeMethod(this, &EventHandler::handleStructureRemovedEvent);
                return pipe;
            }
        };
        EventHandler m_ircodeEventHandler;
    public:
        ClassResearcher(
            ircode::Program* program,
            ClassRepository* classRepo,
            StructureRepository* structureRepo
        )
            : m_program(program)
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

                // set label offsets and labels (see test ClassResearcherTest.UserDefinedLabelFieldOffset)
                auto labelOffset = m_classRepo->getUserDefinedLabelOffset(structuresInGroup);
                if (labelOffset == -1) {
                    // if no user defined label offset, use 0
                    labelOffset = 0;
                }
                for (auto structure : structuresInGroup) {
                    auto info = m_classRepo->getStructureInfo(structure);
                    info->labelOffset = labelOffset;
                    info->labels = getLabels(info->getLabelSet(), labelOffset);
                }

                ClassFieldRangeSet classFieldRangeSet;
                std::map<size_t, std::map<size_t, std::list<Structure*>>> structuresByFieldAndLabel;
                for (auto structure : structuresInGroup) {
                    auto info = m_classRepo->getStructureInfo(structure);

                    // fill labelsToMaxOffset
                    size_t lastOffset = -1;
                    if (!structure->fields.empty()) {
                        lastOffset = structure->fields.rbegin()->first;
                    }
                    if (!structure->conditions.values().empty()) {
                        lastOffset = std::max(lastOffset, structure->conditions.values().rbegin()->first);
                    }
                    if (lastOffset != -1) {
                        classFieldRangeSet.addRange(info->labels, lastOffset);
                    }

                    // fill structuresByFieldAndLabel (indexing structures by [field, label])
                    for (auto& [offset, field] : structure->fields) {
                        for (auto label : info->labels) {
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
                for (auto groupStruct : info->group->getStructures()) {
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
                for (auto groupStruct : info->group->getStructures()) {
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

        std::set<size_t> getLabels(const ConstantSet& set, size_t labelOffset) {
            auto it = set.values().find(labelOffset);
            if (it != set.values().end()) {
                return it->second;
            }
            return { size_t(-1) };
        }
    };
};
