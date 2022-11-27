#include "SDA/Callbacks/ProjectContextCallbacks.h"
#include "SDA/Program.h"
#include "SDA/Project.h"
#include "SDA/Factory.h"
#include "SDA/Change.h"
#include "SDA/Database/Transaction.h"

using namespace sda;

ProjectContextCallbacks::ProjectContextCallbacks(Project* project, std::shared_ptr<Callbacks> prevCallbacks)
    : m_project(project), m_prevCallbacks(prevCallbacks)
{}

void ProjectContextCallbacks::onObjectAdded(Object* obj) {
    if (m_prevCallbacks)
        m_prevCallbacks->onObjectAdded(obj);

    m_project->getTransaction()->markAsNew(obj);
    if(auto objChange = getOrCreateObjectChange())
        objChange->markAsNew(obj);
}

void ProjectContextCallbacks::onObjectModified(Object* obj) {
    if (m_prevCallbacks)
        m_prevCallbacks->onObjectModified(obj);

    m_project->getTransaction()->markAsModified(obj);
    if(auto objChange = getOrCreateObjectChange())
        objChange->markAsModified(obj);
}

void ProjectContextCallbacks::onObjectRemoved(Object* obj) {
    if (m_prevCallbacks)
        m_prevCallbacks->onObjectRemoved(obj);

    m_project->getTransaction()->markAsRemoved(obj);
    if(auto objChange = getOrCreateObjectChange())
        objChange->markAsRemoved(obj);
}

ObjectChange* ProjectContextCallbacks::getOrCreateObjectChange() const {
    auto changeList = m_project->getChangeChain()->getChangeList();
    if(!changeList)
        return nullptr;
    return changeList->getOrCreateObjectChange(m_project->getFactory());
}