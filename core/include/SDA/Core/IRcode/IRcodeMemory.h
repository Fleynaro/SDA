#pragma once
#include "SDA/Core/IRcode/IRcodeValue.h"

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
    };
};