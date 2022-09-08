#include "Init.h"

void sda::bind::InitModule(v8::Local<v8::Object> module, std::list<std::function<void(v8pp::module&)>> inits) {
    auto isolate = v8::Isolate::GetCurrent();
    v8pp::module m(isolate);
    for (auto& init : inits)
        init(m);
    v8pp::set_option(isolate, module, "exports", m.new_instance());

    // node::AtExit(nullptr, [](void* param) {
    //     v8pp::cleanup(static_cast<v8::Isolate*>(param));
    // }, isolate);
}