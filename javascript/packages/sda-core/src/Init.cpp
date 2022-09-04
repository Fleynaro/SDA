#include "node.h"
#include "Core/ContextBind.h"
#include "Core/DataType/DataTypeBind.h"
#include "Core/DataType/VoidDataTypeBind.h"

using namespace sda::bind;

void InitAllClasses(v8pp::module& m) {
    std::list<std::function<void(v8pp::module&)>> initList = {
        ContextCallbacksBind::Init,
        ContextBind::Init,
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
}

NODE_MODULE(core, InitModule)