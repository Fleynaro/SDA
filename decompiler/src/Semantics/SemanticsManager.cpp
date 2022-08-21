#include "Decompiler/Semantics/SemanticsManager.h"

using namespace sda;
using namespace sda::decompiler;

SemanticsManager::SemanticsManager(Context* context)
    : m_context(context)
{}

Context* SemanticsManager::getContext() const {
    return m_context;
}

void SemanticsManager::removeObject(SemanticsObject* object, SemanticsContextOperations& operations) {
    auto semanitcs = object->findSemantics(Semantics::FilterAll());
    for (auto semToRemove : semanitcs)
        removeSemantics(semToRemove, operations);
    m_objects.erase(object->getId());
}

SemanticsObject* SemanticsManager::getObject(SemanticsObject::Id id) const {
    auto it = m_objects.find(id);
    if (it != m_objects.end())
        return it->second.get();
    return nullptr;
}

void SemanticsManager::removeSemantics(Semantics* semantics, SemanticsContextOperations& operations) {
    std::list<Semantics*> semanticsToRemove = {semantics};
    std::list<SemanticsObject*> sourceHolders;
    while (!semanticsToRemove.empty()) {
        auto semToRemove = semanticsToRemove.front();
        semanticsToRemove.pop_front();
        
        if (semToRemove->getPredecessors().size() >= 2 || semToRemove == semantics)
            sourceHolders.push_back(semToRemove->getHolder());

        for (auto& nextSem : semToRemove->getSuccessors())
            semanticsToRemove.push_back(nextSem);

        delete semToRemove;
    }

    for (auto holder : sourceHolders)
        holder->getAllRelatedOperations(operations);
}

bool SemanticsManager::isSimiliarityConsidered() const {
    return false;
}

void SemanticsManager::propagate(SemanticsContextOperations& operations) {
    while (!operations.empty()) {
        auto it = operations.begin();
        auto op = *it;

        for (auto& propagator : m_propagators)
            propagator->propagate(op.context, op.operation, operations);

        operations.erase(it);
    }
}