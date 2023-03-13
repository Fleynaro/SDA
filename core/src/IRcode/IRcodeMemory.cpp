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
