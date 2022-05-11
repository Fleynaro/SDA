#include "Callbacks/ContextCallbacks.h"
#include "Project.h"

using namespace sda;

ContextCallbacks::ContextCallbacks(Context* context, Project* project)
    : m_project(project)
{
    context->setCallbacks(std::unique_ptr<Callbacks>(this));
}

void ContextCallbacks::add(std::unique_ptr<Callbacks> callback) {
    m_callbacks.push_back(std::move(callback));
}

void ContextCallbacks::onObjectAdded(IObject* obj) {
    for (const auto& callback : m_callbacks)
        callback->onObjectAdded(obj);
}

void ContextCallbacks::onObjectRemoved(IObject* obj) {
    for (const auto& callback : m_callbacks)
        callback->onObjectRemoved(obj);
}

void ContextCallbacks::onObjectModified(IObject* obj) {
    for (const auto& callback : m_callbacks)
        callback->onObjectModified(obj);
}