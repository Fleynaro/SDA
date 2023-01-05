#include "SDA/Callbacks/ProjectContextCallbacks.h"
#include "SDA/Program.h"
#include "SDA/Project.h"
#include "SDA/Factory.h"
#include "SDA/Change.h"
#include "SDA/Database/Transaction.h"

using namespace sda;

ProjectContextCallbacks::ProjectContextCallbacks(Project* project)
    : m_project(project)
{}

std::string ProjectContextCallbacks::getName() const {
    return "Project";
}

void ProjectContextCallbacks::onObjectAdded(Object* obj) {
    Context::Callbacks::onObjectAdded(obj);

    if (m_transactionEnabled) {
        m_project->getTransaction()->markAsNew(obj);
    }
    if (m_changeEnabled) {
        if(auto objChange = getOrCreateObjectChange()) {
            objChange->markAsNew(obj);
        }
    }
}

void ProjectContextCallbacks::onObjectModified(Object* obj) {
    Context::Callbacks::onObjectModified(obj);

    if (m_transactionEnabled) {
        m_project->getTransaction()->markAsModified(obj);
    }
    if (m_changeEnabled) {
        if(auto objChange = getOrCreateObjectChange()) {
            objChange->markAsModified(obj);
        }
    }
}

void ProjectContextCallbacks::onObjectRemoved(Object* obj) {
    Context::Callbacks::onObjectRemoved(obj);

    if (m_transactionEnabled) {
        m_project->getTransaction()->markAsRemoved(obj);
    }
    if (m_changeEnabled) {
        if(auto objChange = getOrCreateObjectChange()) {
            objChange->markAsRemoved(obj);
        }
    }
}

ObjectChange* ProjectContextCallbacks::getOrCreateObjectChange() const {
    auto changeList = m_project->getChangeChain()->getChangeList();
    if(!changeList)
        return nullptr;
    return changeList->getOrCreateObjectChange(m_project->getFactory());
}