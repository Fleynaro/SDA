#include "SDA/Core/Event/EventPipe.h"
#include <gtest/gtest.h>

using namespace sda;

static const size_t TestEventTopic = TopicName("TestEventTopic");

struct TestEvent : Event {
    size_t value;
    TestEvent(size_t value)
        : Event(TestEventTopic)
        , value(value)
    {}
};

std::function<void(const TestEvent&)> Handler(size_t& result) {
    return [&](const TestEvent& e) {
        result += e.value;
    };
}

TEST(EventPipeTest, sendEvent) {
    auto pipe = EventPipe::New();
    size_t result = 0;
    pipe->subscribe(Handler(result));
    pipe->send(TestEvent(1));
    EXPECT_EQ(result, 1);
}

TEST(EventPipeTest, unsubscribe) {
    auto pipe = EventPipe::New();
    size_t result = 0;
    auto unsubscribe = pipe->subscribe(Handler(result));
    unsubscribe();
    pipe->send(TestEvent(1));
    EXPECT_EQ(result, 0);
}

TEST(EventPipeTest, nested) {
    auto pipe = EventPipe::New();
    size_t result = 0;
    pipe->subscribe(Handler(result));
    pipe->subscribe(std::function([&](const TestEvent& e) {
        if (e.value <= 3) {
            // send another event inside event handler
            pipe->send(TestEvent(e.value + 1));
        }
    }));
    pipe->send(TestEvent(1));
    EXPECT_EQ(result, 10); // 1 + 2 + 3 + 4
}

TEST(EventPipeTest, filter) {
    auto pipe = EventPipe::New();
    size_t result = 0;
    pipe
        ->filter(std::function([](const Event& e) {
            return e.topic == TestEventTopic;
        }))
        ->subscribe(Handler(result));
    pipe->send(TestEvent(1));
    EXPECT_EQ(result, 1);
}

TEST(EventPipeTest, process) {
    auto pipe = EventPipe::New();
    size_t result = 0;
    pipe
        ->process(std::function([](const Event& e, const EventNext& next) {
            // modify event by adding 1 to value
            if (auto event = dynamic_cast<const TestEvent*>(&e))
                next(TestEvent(event->value + 1));
        }))
        ->subscribe(Handler(result));
    pipe->send(TestEvent(1));
    EXPECT_EQ(result, 2);
}

TEST(EventPipeTest, connect) {
    auto pipe1 = EventPipe::New();
    auto pipe2 = EventPipe::New();
    size_t result = 0;
    pipe2->subscribe(Handler(result));
    pipe1->connect(pipe2);
    pipe1->send(TestEvent(1));
    EXPECT_EQ(result, 1);
}

TEST(EventPipeTest, disconnect) {
    auto pipe1 = EventPipe::New();
    auto pipe2 = EventPipe::New();
    size_t result = 0;
    pipe2->subscribe(Handler(result));
    pipe1->connect(pipe2);
    pipe1->disconnect(pipe2);
    pipe1->send(TestEvent(1));
    EXPECT_EQ(result, 0);
}

TEST(EventPipeTest, combine) {
    // create input pipe
    auto pipeIn = EventPipe::New();
    auto pipeOut = pipeIn
        // create and connect filter pipe
        ->filter(std::function([](const Event& e) {
            return e.topic == TestEventTopic;
        }))
        // create and connect process pipe
        ->process(std::function([](const Event& e, const EventNext& next) {
            // modify event by adding 1 to value
            if (auto event = dynamic_cast<const TestEvent*>(&e))
                next(TestEvent(event->value + 1));
        }));
    // you can combine multiple pipes into one
    auto pipe = EventPipe::Combine(pipeIn, pipeOut);
    size_t result = 0;
    pipe->subscribe(Handler(result));
    pipe->send(TestEvent(1));
    EXPECT_EQ(result, 2);
}

TEST(EventPipeTest, condition) {
    auto pipeThen = EventPipe::New();
    auto pipeElse = EventPipe::New();
    auto pipeIf = EventPipe::If(
        std::function([](const Event& e) {
            if (auto event = dynamic_cast<const TestEvent*>(&e))
                return event->value % 2 == 0;
            return false;
        }),
        pipeThen, // pipeThen will receive events with even values
        pipeElse // otherwise pipeElse will receive events
    );
    size_t resultThen = 0;
    size_t resultElse = 0;
    pipeThen->subscribe(Handler(resultThen));
    pipeElse->subscribe(Handler(resultElse));
    pipeIf->send(TestEvent(1));
    pipeIf->send(TestEvent(2));
    EXPECT_EQ(resultThen, 2);
    EXPECT_EQ(resultElse, 1);
}