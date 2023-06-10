#pragma once
#include <string>

namespace sda
{
    struct Event {
        size_t topic;

        Event(size_t topic);

        virtual ~Event() = default;
    };

    size_t TopicName(const std::string& name);
};