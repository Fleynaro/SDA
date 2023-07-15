#pragma once
#include "StructureResearcher.h"

namespace sda::researcher
{
    class ClassRepository
    {
        struct StructureInfo {
            ConditionSet conditions;
        };
        std::map<Structure*, StructureInfo> m_structureToInfo;
        std::shared_ptr<EventPipe> m_eventPipe;
    public:
        ClassRepository(std::shared_ptr<EventPipe> eventPipe)
            : m_eventPipe(eventPipe)
        {}

        StructureInfo* getStructureInfo(Structure* structure) {
            auto it = m_structureToInfo.find(structure);
            if (it == m_structureToInfo.end()) {
                it = m_structureToInfo.emplace(structure, StructureInfo()).first;
            }
            return &it->second;
        }

        void removeStructure(Structure* structure) {
            m_structureToInfo.erase(structure);
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

            void handleStructureUpdatedEvent(const StructureUpdatedEvent& event) {
                m_researcher->research(event.structure);
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
                    while (!data->updatedStructures.empty()) {
                        auto structure = data->updatedStructures.front();
                        data->updatedStructures.pop_front();
                        next(StructureUpdatedEvent(structure));
                    }
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
                    ->subscribeMethod(this, &EventHandler::handleStructureUpdatedEvent);
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

        void research(Structure* startStructure) {
            startStructure->passDescendants(std::function([this](Structure* structure, bool& goNext) {
                research(structure, goNext);
            }));
        }

    private:
        void research(Structure* structure, bool& goNext) {
            auto info = m_classRepo->getStructureInfo(structure);
            auto conditionHash = info->conditions.hash();
            for (auto input : structure->inputs) {
                info->conditions.merge(input->conditions);
            }
            info->conditions.merge(structure->conditions, true);
            goNext = info->conditions.hash() != conditionHash;
        }
    };
};
