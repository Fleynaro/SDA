#include "SDA/Core/Commit.h"
#include <gtest/gtest.h>

using namespace sda;

static const size_t TestEventTopic = TopicName("TestEventTopic");

struct TestEvent : Event {
    size_t value;
    TestEvent(size_t value = 0)
        : Event(TestEventTopic)
        , value(value)
    {}
};

std::function<void(const TestEvent&)> Handler_(size_t& result) {
    return [&](const TestEvent& e) {
        result = e.value;
    };
}

TEST(CommitTest, commitEvents) {
    auto pipe = EventPipe::New();
    size_t result = 0;
    pipe->subscribe(std::function([&](const CommitBeginEvent& e) {
        result ++;
    }));
    pipe->subscribe(std::function([&](const CommitEndEvent& e) {
        result ++;
    }));
    {
        CommitScope commit(pipe);
        pipe->send(TestEvent(10));
        {
            CommitScope commit(pipe);
            pipe->send(TestEvent(20));
        }
    }
    // CommitBeginEvent (+1)
    // TestEvent(10)
    // CommitBeginEvent (+1)
    // TestEvent(20)
    // CommitEndEvent   (+1)
    // CommitEndEvent   (+1)
    EXPECT_EQ(result, 4);
}

TEST(CommitTest, commitEventsWithCommitPipe) {
    auto pipe = EventPipe::New();
    auto commitPipe = pipe->connect(CommitPipe());
    size_t result = 0;
    commitPipe->subscribe(std::function([&](const CommitBeginEvent& e) {
        result ++;
    }));
    commitPipe->subscribe(std::function([&](const CommitEndEvent& e) {
        result ++;
    }));
    {
        CommitScope commit(pipe);
        pipe->send(TestEvent(10));
        {
            CommitScope commit(pipe);
            pipe->send(TestEvent(20));
        }
    }
    // CommitBeginEvent (+1)
    // TestEvent(10)
    // TestEvent(20)
    // CommitEndEvent   (+1)
    EXPECT_EQ(result, 2);
}

TEST(CommitTest, commitEventsWithMultipleCommitPipe) {
    auto pipe = EventPipe::New();
    std::list<size_t> result;
    // notice that CommitEndEvent behaves differently than TestEvent: TestEvent is always ingested by BOTH pipes
    pipe
        // this pipe has the highest priority
        ->connect(CommitPipe())
        ->subscribe(std::function([&](const CommitEndEvent& e) {
            if (result.size() == 0) {
                CommitScope commit(pipe);
                pipe->send(TestEvent());
                result.push_back(1);
                // CommitEndEvent is emitted here and sent to the FIRST pipe ONLY (step 2)
            }
            else if (result.size() == 1) {
                result.push_back(2);
            }
            else if (result.size() == 2) {
                result.push_back(3);
            }
            else {
                throw std::runtime_error("should not be called");
            }
        }));
    pipe
        ->connect(CommitPipe())
        ->subscribe(std::function([&](const CommitEndEvent& e) {
            if (result.size() == 2) {
                {
                    CommitScope commit(pipe);
                    pipe->send(TestEvent());
                    // CommitEndEvent is emitted here and sent to BOTH pipes (steps 3, 4)
                }
                result.push_back(5);
            } else {
                result.push_back(4);
            }
        }));
    {
        // start here
        CommitScope commit(pipe);
        pipe->send(TestEvent());
        // CommitEndEvent is emitted here and sent to BOTH pipes
    }
    EXPECT_EQ(result, std::list<size_t>({ 1, 2, 3, 4, 5 }));
}

std::shared_ptr<EventPipe> CreateOptimizedCommitPipe(const EventFilter& filter = EventPipe::FilterTrue) {
    struct Data {
        std::list<size_t> values;
    };
    auto data = std::make_shared<Data>();
    auto commitEndHandler = std::function([data](const EventNext& next) {
        // called when commit is done (after CommitEndEvent)
        auto& values = data->values;
        while(!values.empty()) {
            auto it = values.begin();
            auto value = *it;
            values.erase(it);
            // send aggregated event
            next(TestEvent(value));
        }
    });
    auto [optPipe, commitPipeIn, pipeOut] = OptimizedCommitPipe(commitEndHandler, filter);
    commitPipeIn->subscribe(std::function([data, pipeOut](const TestEvent& e) {
        if (e.value == 1000) {
            // events with value = 1000 will be handled immediately (outside of a commit)
            pipeOut->send(TestEvent(1000));
            return;
        }

        // called when TestEvent is sent within a commit
        auto& values = data->values;
        auto it = std::find(values.begin(), values.end(), e.value);
        if(it == values.end()) { // do not add duplicates (it is agg function)
            values.push_back(e.value);
        }
    }));
    return optPipe;
}

TEST(CommitTest, aggregateEvents) {
    auto pipe = CreateOptimizedCommitPipe();

    // test the pipe
    size_t result = 0;
    pipe->subscribe(std::function([&](const TestEvent& e) {
        result += e.value;
    }));
    {
        // within a commit
        CommitScope commit(pipe);
        pipe->send(TestEvent(10));
        {
            CommitScope commit(pipe);
            pipe->send(TestEvent(10)); // ignored
            pipe->send(TestEvent(20));
        }
    }
    {
        // outside of a commit
        pipe->send(TestEvent(1));
        pipe->send(TestEvent(1)); // not ignored
    }
    // CommitBeginEvent
    // CommitEndEvent
    // TestEvent(10)
    // TestEvent(20)
    // TestEvent(1)
    // TestEvent(1)
    EXPECT_EQ(result, 32);
}

TEST(CommitTest, dontAggregateImmediateEvents) {
    // By "immediate events" we mean events that must be handled immediately (e.g. "removed events")
    auto pipe = CreateOptimizedCommitPipe();

    // test the pipe
    size_t result = 0;
    pipe->subscribe(std::function([&](const TestEvent& e) {
        result += e.value;
    }));
    {
        // within a commit
        CommitScope commit(pipe);
        pipe->send(TestEvent(10));
        pipe->send(TestEvent(10)); // ignored
        pipe->send(TestEvent(1000)); // "immediate event" that will not be stored anywhere and will be handled immediately (outside of a commit)
        pipe->send(TestEvent(1000)); // not ignored (because it is "immediate event")
    }
    // CommitBeginEvent
    // TestEvent(1000)
    // TestEvent(1000)
    // CommitEndEvent
    // TestEvent(10)
    EXPECT_EQ(result, 2010);
}

TEST(CommitTest, aggregateEventsNestedNotIgnored) {
    auto pipe = CreateOptimizedCommitPipe();
    size_t result = 0;
    pipe->subscribe(std::function([&](const TestEvent& e) {
        if (e.value == 10) {
            pipe->send(TestEvent(20)); // not ignored (outside of a commit)
        }
        result += e.value;
    }));
    {
        CommitScope commit(pipe);
        pipe->send(TestEvent(10));
        pipe->send(TestEvent(20));
    }
    // CommitBeginEvent
    // CommitEndEvent
    // TestEvent(10)
    // TestEvent(20)
    // TestEvent(20)
    EXPECT_EQ(result, 50);
}

TEST(CommitTest, aggregateEventsNestedIgnored) {
    auto pipe = CreateOptimizedCommitPipe();
    size_t result = 0;
    pipe->subscribe(std::function([&](const TestEvent& e) {
        if (e.value == 10) {
            CommitScope commitNested(pipe);
            pipe->send(TestEvent(20)); // ignored (within a commit)
        }
        result += e.value;
    }));
    {
        CommitScope commit(pipe);
        pipe->send(TestEvent(10));
        pipe->send(TestEvent(20));
    }
    // CommitBeginEvent
    // CommitEndEvent
    // TestEvent(10)
    // CommitBeginEvent (commitNested)
    // CommitEndEvent (commitNested)
    // TestEvent(20)
    EXPECT_EQ(result, 30);
}

TEST(CommitTest, aggregateEventsMultipleNested) {
    auto pipe = EventPipe::New("test");
    std::list<size_t> result;
    // both pipes ingest all events: 10, 20, 30, 40, 50
    pipe
        // this pipe has the highest priority and will handle all events first
        ->connect(CreateOptimizedCommitPipe())
        ->subscribe(std::function([&](const TestEvent& e) {
            if (e.value == 10) {
                CommitScope commitNested(pipe);
                pipe->send(TestEvent(30));
                pipe->send(TestEvent(20)); // ignored
            }
            else if (e.value == 40) {
                CommitScope commitNested(pipe);
                pipe->send(TestEvent(50));
                pipe->send(TestEvent(50)); // ignored
            }
            else if (e.value == 50) {
                // this event will not be buffered in the first pipe as it's outside a commit
                pipe->send(TestEvent(60));
            }
            // 10, 20, 30, 40, 60, 50
            result.push_back(e.value);
        }));
    pipe
        // this pipe is blocked until the first pipe has handled all events
        ->connect(CreateOptimizedCommitPipe())
        ->subscribe(std::function([&](const TestEvent& e) {
            if (e.value == 20) {
                // if change 'pipe' to the second pipe here, the event (40) will be sent to it only
                CommitScope commitNested(pipe);
                // this event will first be sent to the first pipe (highest priority)
                pipe->send(TestEvent(40));
            }
            // 11, 21, 31, 41, 51, 61
            result.push_back(e.value + 1);
        }));
    {
        // start here
        CommitScope commit(pipe);
        pipe->send(TestEvent(10));
        pipe->send(TestEvent(20));
    }
    EXPECT_EQ(result, (std::list<size_t>{ 10, 20, 30, 11, 40, 60, 50, 21, 31, 41, 51, 61 }));
}

TEST(CommitTest, pipeline) {
    auto pipe = EventPipe::New("test");
    std::list<size_t> result;
    // stage 1 (odd numbers - 11, 21)
    pipe
        ->connect(CreateOptimizedCommitPipe(
            EventPipe::Filter(std::function([&](const TestEvent& e) {
                return e.value % 2 == 1;
            }))
        ))
        ->subscribe(std::function([&](const TestEvent& e) {
            // produce new events as output of stage 1
            pipe->send(TestEvent(20));
            pipe->send(TestEvent(30));
            pipe->send(TestEvent(30)); // ignored
            result.push_back(e.value);
        }));
    // stage 2 (even numbers that generated from stage 1 - 20, 30)
    pipe
        ->connect(CreateOptimizedCommitPipe(
            EventPipe::Filter(std::function([&](const TestEvent& e) {
                return e.value % 2 == 0;
            }))
        ))
        ->subscribe(std::function([&](const TestEvent& e) {
            result.push_back(e.value);
        }));
    {
        // start here
        CommitScope commit(pipe);
        pipe->send(TestEvent(11));
        pipe->send(TestEvent(21));
        pipe->send(TestEvent(11)); // ignored
    }
    EXPECT_EQ(result, (std::list<size_t>{ 11, 21, 20, 30 }));
}
