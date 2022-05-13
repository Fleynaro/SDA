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

void ChangeChain::undo() {
    m_curChangeIt--;
    m_curChangeIt->undo();
}

void ChangeChain::redo() {
    m_curChangeIt++;
    m_curChangeIt->redo();
}

void ChangeChain::newChangeList() {
    m_changes.erase(m_curChangeIt, m_changes.end());
    m_changes.emplace_back();
    m_curChangeIt = m_changes.end();
}

ChangeList* ChangeChain::getChangeList() {
    return &*std::prev(m_curChangeIt);
}