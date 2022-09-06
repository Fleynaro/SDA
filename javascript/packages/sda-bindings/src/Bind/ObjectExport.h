#pragma once
#include <v8pp/class.hpp>

namespace sda::bind
{
    /// As ExportObjectRef but delete memory for C++ object
    template<typename T>
    static auto ExportObject(T* obj) {
        auto isolate = v8::Isolate::GetCurrent();
        return v8pp::class_<T>::import_external(isolate, obj);
    }

    /// As ExportObject but for shared_ptr
    template<typename T>
    static auto ExportObject(std::shared_ptr<T> obj) {
        auto isolate = v8::Isolate::GetCurrent();
        return v8pp::class_<T, v8pp::shared_ptr_traits>::import_external(isolate, obj);
    }

    /// Create JavaScript object which references externally created C++ class.
	/// It will not take ownership of the C++ pointer.
    template<typename T, typename Traits = v8pp::raw_ptr_traits>
    static auto ExportObjectRef(T* obj) {
        auto isolate = v8::Isolate::GetCurrent();
        return v8pp::class_<T, Traits>::reference_external(isolate, obj);
    }

    /// Remove external reference from JavaScript
    template<typename T, typename Traits = v8pp::raw_ptr_traits>
    static auto RemoveObjectRef(T* obj) {
        auto isolate = v8::Isolate::GetCurrent();
        return v8pp::class_<T, Traits>::unreference_external(isolate, obj);
    }
};