#include "node.h"
#include <v8pp/json.hpp>
#include <v8pp/module.hpp>
#include <v8pp/object.hpp>
#include "Bind/ObjectExport.h"
#include "Bind/Call.h"
#include "Bind/Platform/PlatformX86.h"

using namespace sda::bind;

void InitAllClasses(v8pp::module& m) {
    std::list<std::function<void(v8pp::module&)>> initList = {
        PlatformX86Bind::Init
    };
    
    for (auto& init : initList)
        init(m);
}

void InitModule(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
    auto isolate = v8::Isolate::GetCurrent();
    v8pp::module m(isolate);
    InitAllClasses(m);
    v8pp::set_option(isolate, module, "exports", m.new_instance());

    // node::AtExit(nullptr, [](void* param) {
    //     v8pp::cleanup(static_cast<v8::Isolate*>(param));
    // }, isolate);
}

NODE_MODULE(platform_x86, InitModule)