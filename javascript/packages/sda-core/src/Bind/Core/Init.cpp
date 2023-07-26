#include "Bind/Init.h"
#include "Bind/Core/Shared/Converter.h"
#include "Bind/Core/Utils.h"
#include "Bind/Core/Event.h"
#include "Bind/Core/Pcode.h"
#include "Bind/Core/Platform.h"
#include "Bind/Core/Context.h"
#include "Bind/Core/Object.h"
#include "Bind/Core/AddressSpace.h"
#include "Bind/Core/Image.h"
#include "Bind/Core/DataType.h"
#include "Bind/Core/Symbol.h"
#include "Bind/Core/SymbolTable.h"

using namespace sda::bind;

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
    InitModule(module, {
        // utils
        SerializationBind::Init,
        AbstractPrinterBind::Init,
        // event
        EventBind::Init,
        EventPipeBind::Init,
        // p-code
        PcodeVarnodeBind::Init,
        PcodeInstructionBind::Init,
        PcodeBlockBind::Init,
        PcodeFunctionGraphBind::Init,
        PcodeGraphCallbacksBind::Init,
        PcodeGraphBind::Init,
        PcodeParserBind::Init,
        PcodePrinterBind::Init,
        // platform
        PlatformBind::Init,
        PlatformMockBind::Init,
        RegisterRepositoryBind::Init,
        PcodeDecoderBind::Init,
        InstructionDecoderBind::Init,
        CallingConventionBind::Init,
        CustomCallingConventionBind::Init,
        // context
        ContextCallbacksBind::Init,
        ContextBind::Init,
        // object
        ObjectBind::Init,
        ContextObjectBind::Init,
        // image
        AddressSpaceBind::Init,
        ImageRWBind::Init,
        ImageAnalyserBind::Init,
        ImageSectionBind::Init,
        ImageBind::Init,
        // data types
        DataTypeBind::Init,
        VoidDataTypeBind::Init,
        PointerDataTypeBind::Init,
        ArrayDataTypeBind::Init,
        ScalarDataTypeBind::Init,
        TypedefDataTypeBind::Init,
        EnumDataTypeBind::Init,
        StructureDataTypeBind::Init,
        SignatureDataTypeBind::Init,
        // symbols
        SymbolBind::Init,
        VariableSymbolBind::Init,
        FunctionSymbolBind::Init,
        FunctionParameterSymbolBind::Init,
        StructureFieldSymbolBind::Init,
        // symbol tables
        SymbolTableBind::Init,
        StandartSymbolTableBind::Init,
        OptimizedSymbolTableBind::Init,
    });
}

NODE_MODULE(core, Init)