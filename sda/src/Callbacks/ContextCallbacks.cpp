#include "SDA/Callbacks/ContextCallbacks.h"
#include "SDA/Program.h"
#include "SDA/Project.h"
#include "SDA/Factory.h"
#include "SDA/Change.h"
#include "SDA/Database/Transaction.h"

using namespace sda;

ContextCallbacks::ContextCallbacks(Context* context, Project* project)
    : m_project(project)
{
    context->setCallbacks(std::unique_ptr<Callbacks>(this));
}

void ContextCallbacks::add(std::unique_ptr<Callbacks> callback) {
    m_callbacks.push_back(std::move(callback));
}

void ContextCallbacks::onObjectAdded(Object* obj) {
    for (const auto& callback : m_callbacks)
        callback->onObjectAdded(obj);

    m_project->getTransaction()->markAsNew(obj);
    if(auto objChange = getOrCreateObjectChange())
        objChange->markAsNew(obj);
}

void ContextCallbacks::onObjectModified(Object* obj) {
    for (const auto& callback : m_callbacks)
        callback->onObjectModified(obj);

    m_project->getTransaction()->markAsModified(obj);
    if(auto objChange = getOrCreateObjectChange())
        objChange->markAsModified(obj);
}

void ContextCallbacks::onObjectRemoved(Object* obj) {
    for (const auto& callback : m_callbacks)
        callback->onObjectRemoved(obj);

    m_project->getTransaction()->markAsRemoved(obj);
    if(auto objChange = getOrCreateObjectChange())
        objChange->markAsRemoved(obj);
}

ObjectChange* ContextCallbacks::getOrCreateObjectChange() const {
    auto changeList = m_project->getChangeChain()->getChangeList();
    if(!changeList)
        return nullptr;
    return changeList->getOrCreateObjectChange(m_project->getFactory());
}