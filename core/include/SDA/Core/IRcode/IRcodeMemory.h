#pragma once
#include "SDA/Core/IRcode/IRcodeOperation.h"

namespace sda::ircode
{
    struct MemorySubspace {
        std::list<std::shared_ptr<Variable>> variables;
    };

    class MemorySpace
    {
        std::map<Hash, MemorySubspace> m_subspaces;
    public:
        MemorySubspace* getSubspace(Hash baseAddrHash);

        std::shared_ptr<Variable> findVariable(const RefOperation::Reference& reference);

        void removeVariable(std::shared_ptr<Variable> variable);

        void clear();
    };
};