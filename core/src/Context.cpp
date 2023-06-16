#include "SDA/Core/Context.h"
#include "SDA/Core/ContextEvents.h"
#include "SDA/Core/Commit.h"
#include "SDA/Core/Image/AddressSpace.h"
#include "SDA/Core/Image/Image.h"
#include "SDA/Core/DataType/DataType.h"
#include "SDA/Core/Symbol/Symbol.h"
#include "SDA/Core/SymbolTable/SymbolTable.h"
#include "SDA/Core/Utils/Logger.h"

using namespace sda;

Context::Context(std::shared_ptr<EventPipe> eventPipe, Platform* platform)
    : m_eventPipe(eventPipe)
    , m_platform(platform)
{
    m_addressSpaces = std::make_unique<AddressSpaceList>(this);
    m_images = std::make_unique<ImageList>(this);
    m_dataTypes = std::make_unique<DataTypeList>(this);
    m_symbols = std::make_unique<SymbolList>(this);
    m_symbolTables = std::make_unique<SymbolTableList>(this);

    IF_PLOG(plog::debug) {
        m_eventPipe->subscribe(std::function([&](const ObjectAddedEvent& event) {
            if (auto ctxObj = dynamic_cast<ContextObject*>(event.object)) {
                PLOG_DEBUG << "ObjectAddedEvent: " << ctxObj->getName();
            } else {
                PLOG_DEBUG << "ObjectAddedEvent: " << event.object->serializeId();
            }
        }));
        m_eventPipe->subscribe(std::function([&](const ObjectModifiedEvent& event) {
            if (auto ctxObj = dynamic_cast<ContextObject*>(event.object)) {
                PLOG_DEBUG << "ObjectModifiedEvent: " << ctxObj->getName();
            } else {
                PLOG_DEBUG << "ObjectModifiedEvent: " << event.object->serializeId();
            }
        }));
        m_eventPipe->subscribe(std::function([&](const ObjectRemovedEvent& event) {
            if (auto ctxObj = dynamic_cast<ContextObject*>(event.object)) {
                PLOG_DEBUG << "ObjectRemovedEvent: " << ctxObj->getName();
            } else {
                PLOG_DEBUG << "ObjectRemovedEvent: " << event.object->serializeId();
            }
        }));
    }
}

Context::~Context() {
    m_eventPipe->send(ContextDestroyedEvent(this));
}

void Context::initDefault() {
    m_dataTypes->initDefault();
}

std::shared_ptr<EventPipe> Context::getEventPipe() const {
    return m_eventPipe;
}

Platform* Context::getPlatform() const {
    return m_platform;
}

AddressSpaceList* Context::getAddressSpaces() const {
    return m_addressSpaces.get();
}

ImageList* Context::getImages() const {
    return m_images.get();
}

DataTypeList* Context::getDataTypes() const {
    return m_dataTypes.get();
}

SymbolList* Context::getSymbols() const {
    return m_symbols.get();
}

SymbolTableList* Context::getSymbolTables() const {
    return m_symbolTables.get();
}

std::shared_ptr<EventPipe> Context::CreateOptimizedEventPipe() {
    struct Data {
        bool commit = false;
        bool locked = false;
        std::list<std::unique_ptr<Event>> events;
    };
    auto data = std::make_shared<Data>();
    auto pipeIn = EventPipe::New("Context::OptimizedEventPipe")
        ->filter(std::function([data](const Event& event) {
            return event.topic == CommitEventTopic ||
                    dynamic_cast<const ObjectActionEvent*>(&event);
        }));
    pipeIn
        ->connect(CommitPipe())
        ->subscribe(std::function([data](const CommitBeginEvent& event) {
            data->commit = true;
        }));
    auto commitPipeIn = EventPipe::New();
    auto commitPipeOut = commitPipeIn
        ->connect(CommitPipe())
        ->process(std::function([data](const Event& event, const EventNext& next) {
            if (dynamic_cast<const CommitEndEvent*>(&event)) {
                data->commit = false;
                if (!data->locked) {
                    data->locked = true;
                    while(!data->events.empty()) {
                        auto e = std::move(data->events.front());
                        data->events.pop_front();
                        next(*e);
                    }
                    data->locked = false;
                }
            }
        }));
    commitPipeIn->subscribe(std::function([data](const ObjectAddedEvent& event) {
        data->events.push_back(std::make_unique<ObjectAddedEvent>(event));
    }));
    commitPipeIn->subscribe(std::function([data](const ObjectModifiedEvent& event) {
        data->events.remove_if([&event](const std::unique_ptr<Event>& e) {
            if (auto e2 = dynamic_cast<ObjectModifiedEvent*>(e.get())) {
                return e2->object == event.object;
            }
            return false;
        });
        data->events.push_back(std::make_unique<ObjectModifiedEvent>(event));
    }));
    commitPipeIn->subscribe(std::function([data](const ObjectRemovedEvent& event) {
        bool found = false;
        for (auto it = data->events.rbegin(); it != data->events.rend(); ++it) {
            if (auto e = dynamic_cast<ObjectActionEvent*>(it->get())) {
                if (e->object == event.object) {
                    found = true;
                    break;
                }
            }
        }
        if (found) {
            // remove all events related to this object
            auto it = data->events.begin();
            while (it != data->events.end()) {
                if (auto e = dynamic_cast<ObjectActionEvent*>(it->get())) {
                    if (e->object == event.object) {
                        it = data->events.erase(it);
                    } else {
                        ++it;
                    }
                } else {
                    ++it;
                }
            }
        } else {
            data->events.push_back(std::make_unique<ObjectRemovedEvent>(event));
        }
    }));
    // All input events are moving through pipeIn and filtered:
    // - if event is emitted within some commit, it is handled by commitPipeIn and when commit ends it leaves through commitPipeOut
    // - if event is emitted outside of commit, it immediately leaves pipeIn
    return EventPipe::Combine(
        pipeIn,
        pipeIn->connect(
            EventPipe::If(
                std::function([data](const Event& event) {
                    return data->commit;
                }),
                EventPipe::Combine(commitPipeIn, commitPipeOut),
                EventPipe::New()
            )
        )
    );
}
