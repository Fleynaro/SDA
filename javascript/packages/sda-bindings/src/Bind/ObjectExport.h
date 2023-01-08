#pragma once
#include <v8pp/class.hpp>

namespace sda::bind
{
    /// Create JavaScript object which references externally created C++ class.
    /// It will take ownership of the C++ pointer and delete memory for it
    template<typename T>
    static auto ExportObject(T* obj) {
        auto isolate = v8::Isolate::GetCurrent();
        return v8pp::class_<T, v8pp::raw_ptr_traits>::import_external(isolate, obj);
    }

    /// As ExportObject but for shared_ptr
    template<typename T>
    static auto ExportSharedObject(std::shared_ptr<T> obj) {
        auto isolate = v8::Isolate::GetCurrent();
        return v8pp::class_<T, v8pp::shared_ptr_traits>::import_external(isolate, obj);
    }

    /// Remove external reference from JavaScript
    template<typename T>
    static bool RemoveObjectRef(T* obj) {
        auto isolate = v8::Isolate::GetCurrent();
        return v8pp::class_<T, v8pp::raw_ptr_traits>::unreference_external(isolate, obj);
    }
};