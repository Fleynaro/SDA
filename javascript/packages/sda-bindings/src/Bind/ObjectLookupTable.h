#pragma once
#include <v8pp/class.hpp>
#include "Shared/SharedData.h"

namespace sda::bind
{
    class ObjectLookupTableRaw
    {
        using Hash = size_t;
        struct Fields {
            std::unordered_map<Hash, void*> table;
            bool isRegistered = false;
        };
    public:
        static void AddObject(void* obj) {
            auto fields = getFields();
            if (!fields->isRegistered)
                return;
            fields->table[GetHash(obj)] = obj;
        }
        
        static void RemoveObject(void* obj) {
            auto fields = getFields();
            if (!fields->isRegistered)
                return;
            fields->table.erase(GetHash(obj));
        }

        template<typename T>
        static void Register(v8pp::class_<T, v8pp::raw_ptr_traits>& cl) {
            cl
                .property("hashId", [](T& self) { return GetHash(&self); })
                .static_method("Get", &GetObject<T>);
            getFields()->isRegistered = true;
        }
        
    private:
        static Fields* getFields() {
            void* fields = SharedData::Get<ObjectLookupTableRaw>();
            if (fields == nullptr) {
                fields = new Fields();
                SharedData::Set<ObjectLookupTableRaw>(fields);
            }
            return static_cast<Fields*>(fields);
        }

        static Hash GetHash(void* obj) {
            return Hash(obj);
        }

        template<typename T>
        static T* GetObject(Hash hash) {
            auto fields = getFields();
            auto it = fields->table.find(hash);
            if (it != fields->table.end()) {
                return static_cast<T*>(it->second);
            }
            return nullptr;
        }
    };

    class ObjectLookupTableShared
    {
        using Hash = size_t;
        struct Fields {
            std::unordered_map<Hash, std::weak_ptr<void>> table;
            bool isRegistered = false;
        };
    public:
        static void AddObject(std::shared_ptr<void> obj) {
            auto fields = getFields();
            if (!fields->isRegistered)
                return;
            fields->table[GetHash(obj.get())] = obj;
        }

        static void RemoveObject(std::shared_ptr<void> obj) {
            auto fields = getFields();
            if (!fields->isRegistered)
                return;
            fields->table.erase(GetHash(obj.get()));
        }

        template<typename T>
        static void Register(v8pp::class_<T, v8pp::shared_ptr_traits>& cl) {
            cl
                .property("hashId", [](T& self) { return GetHash(&self); })
                .static_method("Get", &GetObject<T>);
            getFields()->isRegistered = true;
        }

        static void Init(v8pp::module& module) {
            module.function("CleanUpSharedObjectLookupTable", &CleanUp);
        }
        
    private:
        static Fields* getFields() {
            void* fields = SharedData::Get<ObjectLookupTableShared>();
            if (fields == nullptr) {
                fields = new Fields();
                SharedData::Set<ObjectLookupTableShared>(fields);
            }
            return static_cast<Fields*>(fields);
        }

        static Hash GetHash(void* obj) {
            return Hash(obj);
        }

        template<typename T>
        static std::shared_ptr<T> GetObject(Hash hash) {
            auto fields = getFields();
            auto it = fields->table.find(hash);
            if (it != fields->table.end()) {
                return std::static_pointer_cast<T>(it->second.lock());
            }
            return nullptr;
        }

        static void CleanUp() {
            auto fields = getFields();
            for (auto& [hash, obj] : fields->table) {
                if (obj.expired()) {
                    fields->table.erase(hash);
                }
            }
        }
    };
};