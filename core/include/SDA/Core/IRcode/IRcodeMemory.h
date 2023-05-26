#pragma once
#include "SDA/Core/IRcode/IRcodeOperation.h"

namespace sda::ircode
{
    struct MemorySubspace {
        std::list<std::shared_ptr<Variable>> variables;
        std::set<std::shared_ptr<Variable>> blockScopedVars;
    };

    class MemorySpace
    {
        std::map<Hash, MemorySubspace> m_subspaces;
    public:
        MemorySubspace* getSubspace(Hash baseAddrHash);

        std::shared_ptr<Variable> findVariable(const RefOperation::Reference& reference);

        void clear();
    };
};