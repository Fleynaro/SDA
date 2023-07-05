#include "SDA/Core/Semantics/StructureResearcher.h"

using namespace sda;
using namespace sda::semantics;

StructureRepository::StructureRepository(std::shared_ptr<sda::EventPipe> eventPipe)
    : m_eventPipe(eventPipe)
{
    IF_PLOG(plog::debug) {
        m_eventPipe->subscribe(std::function([&](const StructureCreatedEvent& event) {
            PLOG_DEBUG << "StructureCreatedEvent: " << event.structure->name;
        }));
        m_eventPipe->subscribe(std::function([&](const ChildAddedEvent& event) {
            PLOG_DEBUG << "ChildAddedEvent: " << event.structure->name << " -> child " << event.child->name;
        }));
        m_eventPipe->subscribe(std::function([&](const LinkCreatedEvent& event) {
            std::string offsetStr;
            if (event.offset) {
                offsetStr = (std::stringstream() << " + 0x" << utils::to_hex() << event.offset).str();
            }
            PLOG_DEBUG << "LinkCreatedEvent: node " << event.node->getName()
                        << " -> struct " << event.structure->name << offsetStr;
        }));
    }
}