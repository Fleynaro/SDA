#include "Change.h"

using namespace sda;

void ChangeList::undo() {
    for (auto& change : m_changes)
        change->undo();
}

void ChangeList::redo() {
    for (auto& change : m_changes)
        change->redo();
}

void ChangeList::add(std::unique_ptr<IChange> change) {
    m_changes.push_back(std::move(change));
}

ObjectChange* ChangeList::getOrCreateObjectChange(Context* context, IFactory* factory) {
    if(auto objectChange = dynamic_cast<ObjectChange*>(m_changes.back().get()))
        return objectChange;
    auto objectChange = new ObjectChange(context, factory);
    m_changes.push_back(std::unique_ptr<IChange>(objectChange));
    return objectChange;
}

void ChangeChain::undo() {
    m_locked = true;
    m_curChangeIt--;
    m_curChangeIt->undo();
    m_locked = false;
}

void ChangeChain::redo() {
    m_locked = true;
    m_curChangeIt++;
    m_curChangeIt->redo();
    m_locked = false;
}

void ChangeChain::newChangeList() {
    m_changes.erase(m_curChangeIt, m_changes.end());
    m_changes.emplace_back();
    m_curChangeIt = m_changes.end();
}

ChangeList* ChangeChain::getChangeList() {
    return &*std::prev(m_curChangeIt);
}

bool ChangeChain::isLocked() {
    return m_locked;
}

void ObjectChange::undo() {
    // TODO
}

void ObjectChange::redo() {
    // TODO
}

void ObjectChange::markAsNew(ISerializable* obj) {
    auto it = m_changes.find(obj);
    if (it == m_changes.end()) {
        m_changes[obj] = { ObjectChangeData::New };
    }
}

void ObjectChange::markAsModified(ISerializable* obj) {
    
}

void ObjectChange::markAsRemoved(ISerializable* obj) {
    auto it = m_changes.find(obj);
    if (it == m_changes.end()) {
        m_changes[obj] = { ObjectChangeData::New };
    }
}