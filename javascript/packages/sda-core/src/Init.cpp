#include "node.h"
#include <v8pp/json.hpp>
#include <v8pp/module.hpp>
#include <v8pp/object.hpp>
#include "Bind/ObjectExport.h"
#include "Bind/Call.h"
#include "Bind/Core/Utils.h"
#include "Bind/Core/Context.h"
#include "Bind/Core/Object.h"
#include "Bind/Core/DataType.h"
#include "Bind/Core/Symbol.h"
#include "Bind/Core/SymbolTable.h"

using namespace sda::bind;

void ContextBind::BindCallbacks::onObjectAdded(Object* obj) {
    if (dynamic_cast<DataType*>(obj))
    {
        if (auto voidDt = dynamic_cast<VoidDataType*>(obj))
            ExportObjectRef(voidDt);
        else if (auto ptrDt = dynamic_cast<PointerDataType*>(obj))
            ExportObjectRef(ptrDt);
        else if (auto arrDt = dynamic_cast<ArrayDataType*>(obj))
            ExportObjectRef(arrDt);
        else if (auto scalarDt = dynamic_cast<ScalarDataType*>(obj))
            ExportObjectRef(scalarDt);
        else if (auto typedefDt = dynamic_cast<TypedefDataType*>(obj))
            ExportObjectRef(typedefDt);
        else if (auto enumDt = dynamic_cast<EnumDataType*>(obj))
            ExportObjectRef(enumDt);
        else if (auto structDt = dynamic_cast<StructureDataType*>(obj))
            ExportObjectRef(structDt);
        else if (auto sigDt = dynamic_cast<SignatureDataType*>(obj))
            ExportObjectRef(sigDt);
    }
    else if (dynamic_cast<Symbol*>(obj))
    {
        if (auto varSym = dynamic_cast<VariableSymbol*>(obj))
            ExportObjectRef(varSym);
        else if (auto funcSym = dynamic_cast<FunctionSymbol*>(obj))
            ExportObjectRef(funcSym);
        else if (auto paramSym = dynamic_cast<FunctionParameterSymbol*>(obj))
            ExportObjectRef(paramSym);
        else if (auto structFieldSym = dynamic_cast<StructureFieldSymbol*>(obj))
            ExportObjectRef(structFieldSym);
    }
}

void ContextBind::BindCallbacks::onObjectRemoved(Object* obj) {
    if (dynamic_cast<DataType*>(obj))
    {
        if (auto voidDt = dynamic_cast<VoidDataType*>(obj))
            RemoveObjectRef(voidDt);
        else if (auto ptrDt = dynamic_cast<PointerDataType*>(obj))
            RemoveObjectRef(ptrDt);
        else if (auto arrDt = dynamic_cast<ArrayDataType*>(obj))
            RemoveObjectRef(arrDt);
        else if (auto scalarDt = dynamic_cast<ScalarDataType*>(obj))
            RemoveObjectRef(scalarDt);
        else if (auto typedefDt = dynamic_cast<TypedefDataType*>(obj))
            RemoveObjectRef(typedefDt);
        else if (auto enumDt = dynamic_cast<EnumDataType*>(obj))
            RemoveObjectRef(enumDt);
        else if (auto structDt = dynamic_cast<StructureDataType*>(obj))
            RemoveObjectRef(structDt);
        else if (auto sigDt = dynamic_cast<SignatureDataType*>(obj))
            RemoveObjectRef(sigDt);
    }
    else if (dynamic_cast<Symbol*>(obj))
    {
        if (auto varSym = dynamic_cast<VariableSymbol*>(obj))
            RemoveObjectRef(varSym);
        else if (auto funcSym = dynamic_cast<FunctionSymbol*>(obj))
            RemoveObjectRef(funcSym);
        else if (auto paramSym = dynamic_cast<FunctionParameterSymbol*>(obj))
            RemoveObjectRef(paramSym);
        else if (auto structFieldSym = dynamic_cast<StructureFieldSymbol*>(obj))
            RemoveObjectRef(structFieldSym);
    }
}

void InitAllClasses(v8pp::module& m) {
    std::list<std::function<void(v8pp::module&)>> initList = {
        // utils
        SerializationBind::Init,
        // context
        ContextCallbacksBind::Init,
        ContextBind::Init,
        // object
        ObjectBind::Init,
        ContextObjectBind::Init,
        // data types
        DataTypeBind::Init,
        VoidDataTypeBind::Init,
        PointerDataTypeBind::Init,
        ArrayDataTypeBind::Init,
        ScalarDataTypeBind::Init,
        TypedefDataTypeBind::Init,
        EnumDataTypeBind::Init,
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

NODE_MODULE(core, InitModule)