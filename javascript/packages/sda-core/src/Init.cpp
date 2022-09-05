#include "node.h"
#include <v8pp/module.hpp>
#include "Bind/ObjectExport.h"
#include "Bind/Call.h"
#include "Bind/Core/Context.h"
#include "Bind/Core/Object.h"
#include "Bind/Core/DataType.h"

using namespace sda::bind;

void InitAllClasses(v8pp::module& m) {
    std::list<std::function<void(v8pp::module&)>> initList = {
        ContextCallbacksBind::Init,
        ContextBind::Init,
        ObjectBind::Init,
        ContextObjectBind::Init,
        DataTypeBind::Init,
        VoidDataTypeBind::Init
    };
    
    for (auto& init : initList)
        init(m);
}

void InitModule(v8::Local<v8::Object> exports) {
    auto isolate = v8::Isolate::GetCurrent();
    v8pp::module m(isolate);
    InitAllClasses(m);
    exports->SetPrototype(isolate->GetCurrentContext(), m.new_instance());

    // node::AtExit(nullptr, [](void* param) {
    //     v8pp::cleanup(static_cast<v8::Isolate*>(param));
    // }, isolate);
}

NODE_MODULE(core, InitModule)