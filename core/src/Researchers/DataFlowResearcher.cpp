#include "SDA/Core/Researchers/DataFlowResearcher.h"

using namespace sda;
using namespace sda::semantics;

DataFlowRepository::DataFlowRepository(std::shared_ptr<sda::EventPipe> eventPipe)
    : m_eventPipe(eventPipe)
{
    m_globalNode = { DataFlowNode::Start };
    IF_PLOG(plog::debug) {
        m_eventPipe->subscribe(std::function([&](const DataFlowNodeCreatedEvent& event) {
            PLOG_DEBUG << "DataFlowNodeCreatedEvent: " << event.node->getName();
        }));
        m_eventPipe->subscribe(std::function([&](const DataFlowNodeUpdatedEvent& event) {
            PLOG_DEBUG << "DataFlowNodeUpdatedEvent: " << event.node->getName();
        }));
        m_eventPipe->subscribe(std::function([&](const DataFlowNodeRemovedEvent& event) {
            PLOG_DEBUG << "DataFlowNodeRemovedEvent: " << event.node->getName();
        }));
        m_eventPipe->subscribe(std::function([&](const DataFlowNodePassedEvent& event) {
            std::stringstream ss;
            std::string sep;
            for (auto node : event.nextNodes) {
                ss << sep << node->getName();
                sep = ",";
            }
            PLOG_DEBUG << "DataFlowNodePassedEvent: " << event.node->getName() << "(next = " << ss.str() << ")";
        }));
    }
}