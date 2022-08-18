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
    for (auto nextSem : semantics->getSuccessors())
        removeSemantics(nextSem, operations);
    m_semantics.remove_if([semantics](const std::unique_ptr<Semantics>& sem) {
        return sem.get() == semantics;
    });
    
    auto srcSem = semantics->getSourceInfo()->sourceSemantics;
    operations.join(srcSem->getr);
}

void SemanticsManager::addPropagator(std::unique_ptr<SemanticsPropagator> propagator) {
    m_propagators.push_back(std::move(propagator));
}

bool SemanticsManager::isSimiliarityConsidered() const {
    return false;
}

void SemanticsManager::propagate(SemanticsContextOperations operations) {
    while (!operations.empty()) {
        auto it = operations.begin();
        auto op = *it;

        for (auto& propagator : m_propagators)
            propagator->propagate(op.context, op.operation, operations);

        operations.erase(it);
    }
}