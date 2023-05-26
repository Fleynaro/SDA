#include "SDA/Core/IRcode/IRcodeMemory.h"

using namespace sda;
using namespace sda::ircode;

MemorySubspace* MemorySpace::getSubspace(ircode::Hash baseAddrHash) {
    auto it = m_subspaces.find(baseAddrHash);
    if (it == m_subspaces.end()) {
        m_subspaces[baseAddrHash] = MemorySubspace();
        return &m_subspaces[baseAddrHash];
    }
    return &it->second;
}

std::shared_ptr<Variable> MemorySpace::findVariable(const RefOperation::Reference& reference) {
    auto it = m_subspaces.find(reference.baseAddrHash);
    if (it == m_subspaces.end()) {
        return nullptr;
    }
    for (auto& variable : it->second.variables) {
        auto varOffset = variable->getMemAddress().offset;
        auto varSize = variable->getSize();
        if (varOffset == reference.offset && varSize == reference.size) {
            return variable;
        }
    }
    return nullptr;
}

void MemorySpace::clear() {
    m_subspaces.clear();
}
