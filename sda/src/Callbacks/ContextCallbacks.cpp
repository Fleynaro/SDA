#include "Callbacks/ContextCallbacks.h"
#include "Program.h"
#include "Project.h"
#include "Factory.h"
#include "Change.h"
#include "Database/Transaction.h"

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

    if (auto serObj = dynamic_cast<ISerializable*>(obj)) {
        m_project->getTransaction()->markAsNew(serObj);

        if(auto objChange = getOrCreateObjectChange())
            objChange->markAsNew(serObj);
    }
}

void ContextCallbacks::onObjectModified(IObject* obj) {
    for (const auto& callback : m_callbacks)
        callback->onObjectModified(obj);

    if (auto serObj = dynamic_cast<ISerializable*>(obj)) {
        m_project->getTransaction()->markAsModified(serObj);
        
        if(auto objChange = getOrCreateObjectChange())
            objChange->markAsModified(serObj);
    }
}

void ContextCallbacks::onObjectRemoved(IObject* obj) {
    for (const auto& callback : m_callbacks)
        callback->onObjectRemoved(obj);

    if (auto serObj = dynamic_cast<ISerializable*>(obj)) {
        m_project->getTransaction()->markAsRemoved(serObj);

        if(auto objChange = getOrCreateObjectChange())
            objChange->markAsRemoved(serObj);
    }
}

ObjectChange* ContextCallbacks::getOrCreateObjectChange() const {
    auto changeList = m_project->getChangeChain()->getChangeList();
    if(!changeList)
        return nullptr;
    return changeList->getOrCreateObjectChange(
        m_project->getContext(), m_project->getProgram()->getFactory());
}