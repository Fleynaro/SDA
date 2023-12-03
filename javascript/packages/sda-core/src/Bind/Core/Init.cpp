#include "Bind/Init.h"
#include "Bind/Core/Shared/Converter.h"
#include "Bind/Core/Utils.h"
#include "Bind/Core/Helpers.h"
#include "Bind/Core/Event.h"
#include "Bind/Core/Pcode.h"
#include "Bind/Core/IRcode.h"
#include "Bind/Core/Platform.h"
#include "Bind/Core/Context.h"
#include "Bind/Core/Object.h"
#include "Bind/Core/AddressSpace.h"
#include "Bind/Core/Image.h"
#include "Bind/Core/DataType.h"
#include "Bind/Core/Symbol.h"
#include "Bind/Core/SymbolTable.h"
#include "Bind/Core/ConstConditionResearcher.h"
#include "Bind/Core/SignatureResearcher.h"
#include "Bind/Core/DataFlowResearcher.h"
#include "Bind/Core/StructureResearcher.h"
#include "Bind/Core/ClassResearcher.h"
#include "Bind/Core/SemanticsResearcher.h"

using namespace sda::bind;

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
    InitModule(module, {
        UtilsBindInit,
        HelpersBindInit,
        ObjectLookupTableShared::Init,
        EventBindInit,
        PlatformBindInit,
        ContextBindInit,
        ObjectBindInit,
        DataTypeBindInit,
        SymbolBindInit,
        SymbolTableBindInit,
        AddressSpaceBindInit,
        ImageBindInit,
        PcodeBindInit,
        IRcodeBindInit,
        ConstConditionBindInit,
        SignatureBindInit,
        DataFlowBindInit,
        StructureBindInit,
        ClassBindInit,
        SemanticsBindInit,
    });
}

NODE_MODULE(core, Init)