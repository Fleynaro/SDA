#include "Bind/Init.h"
#include "Bind/SDA/Program.h"
#include "Bind/SDA/Project.h"
#include "Bind/SDA/Change.h"
#include "Bind/SDA/Factory.h"
#include "Bind/SDA/Database.h"

using namespace sda::bind;

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
    InitModule(module, {
        ProgramBind::Init,
        ProjectBind::Init
    });
}

NODE_MODULE(sda, Init)