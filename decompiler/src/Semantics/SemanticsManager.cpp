#include "Decompiler/Semantics/SemanticsManager.h"

using namespace sda;
using namespace sda::decompiler;

SemanticsManager::SemanticsManager(Context* context)
    : m_context(context)
{}

Context* SemanticsManager::getContext() const {
    return m_context;
}

SemanticsObject* SemanticsManager::getObject(SemanticsObject::Id id) const {
    auto it = m_objects.find(id);
    if (it != m_objects.end())
        return it->second.get();
    return nullptr;
}

void SemanticsManager::removeObject(SemanticsObject* object, SemanticsContextOperations& operations) {
    auto semanitcs = object->findSemantics(Semantics::FilterAll());
    for (auto sem : semanitcs)
        object->removeSemantics(sem, operations);
    object->disconnect();
    m_objects.erase(object->getId());
}

bool SemanticsManager::isSimiliarityConsidered() const {
    return false;
}

void SemanticsManager::propagate(SemanticsContextOperations& operations) {
    auto selectedOps = operations;
    while (!selectedOps.empty()) {
        auto it = selectedOps.begin();
        auto op = *it;

        for (auto& propagator : m_propagators)
            propagator->propagate(op.context, op.operation, operations);

        selectedOps.erase(it);
        operations.erase(op);
    }
}

void SemanticsManager::propagateThroughly(SemanticsContextOperations& operations) {
    while (!operations.empty()) {
        propagate(operations);
    }
}

void SemanticsManager::print(std::ostream& out) {
    std::list<SemanticsObject*> objects;
    for (auto& pair : m_objects)
        objects.push_back(pair.second.get());
    
    objects.sort([](const SemanticsObject* a, const SemanticsObject* b) {
        return a->getName() < b->getName();
    });

    for (auto obj : objects) {
        if (obj->findSemantics(Semantics::FilterAll()).empty())
            continue;
        obj->print(out);
        out << std::endl;
    }
}