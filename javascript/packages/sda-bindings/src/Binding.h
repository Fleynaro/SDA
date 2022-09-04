#pragma once
#include <map>
#include <v8pp/class.hpp>
#include <v8pp/module.hpp>

namespace sda::bind
{
    class Binding
    {
        using Hash = size_t;
        template<typename T>
        Hash hash(const T* obj) {
            return reinterpret_cast<Hash>(obj);
        }

        Hash m_hash;
        static std::map<size_t, Binding*> Bindings;
    public:
        template<typename T, typename R>
        static T* Get(R* obj) {
            auto it = Bindings.find(hash(obj));
            if (it != Bindings.end()) {
                if (auto obj = dynamic_cast<T*>(it->second))
                    return obj;
            }
            return nullptr;
        } 
    protected:
        virtual ~Binding() {
            Bindings.erase(m_hash);
        }

        template<typename T>
        void init(T* obj) {
            m_hash = hash(obj);
            Bindings[m_hash] = this;
        }
    };
};