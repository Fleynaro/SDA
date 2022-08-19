#include "Decompiler/Semantics/SemanticsManager.h"

using namespace sda;
using namespace sda::decompiler;

SemanticsManager::SemanticsManager(Context* context)
    : m_context(context)
{}

Context* SemanticsManager::getContext() const {
    return m_context;
}

SemanticsObject* SemanticsManager::addObject(std::unique_ptr<SemanticsObject> object) {
    auto pObject = object.get();
    m_objects[object->getId()] = std::move(object);
    return pObject;
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

Semantics* SemanticsManager::addSemantics(std::unique_ptr<Semantics> semantics) {
    auto pSem = semantics.get();
    m_semantics.push_back(std::move(semantics));
    return pSem;
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

        m_semantics.remove_if([semToRemove](const std::unique_ptr<Semantics>& sem) {
            return sem.get() == semToRemove;
        });
    }

    for (auto holder : sourceHolders)
        holder->getAllRelatedOperations(operations);
}

void SemanticsManager::addPropagator(std::unique_ptr<SemanticsPropagator> propagator) {
    m_propagators.push_back(std::move(propagator));
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