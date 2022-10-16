#pragma once

namespace utils
{
    using ObjectHash = size_t;

    // Interface for objects that can be hashed
    class IHashable
    {
    public:
        virtual ObjectHash getHash() const = 0;
    };

    template<typename T>
    ObjectHash hash(const T* obj) {
        if (auto hashable = dynamic_cast<const IHashable*>(obj))
            return hashable->getHash();
        return reinterpret_cast<ObjectHash>(obj);
    }

    template<typename T>
    ObjectHash hash(const T& obj) {
        return hash(&obj);
    }
};