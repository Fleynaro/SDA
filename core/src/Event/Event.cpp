#include "SDA/Core/Event/Event.h"

using namespace sda;

Event::Event(size_t topic)
    : topic(topic)
{}

size_t sda::TopicName(const std::string& name) {
    return std::hash<std::string>{}(name);
}
