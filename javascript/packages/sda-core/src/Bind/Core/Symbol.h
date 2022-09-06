#pragma once
#include "Core/Symbol/VariableSymbol.h"
#include "Core/Symbol/FunctionSymbol.h"
#include "Core/Symbol/FunctionParameterSymbol.h"
#include "Core/Symbol/StructureFieldSymbol.h"

namespace sda::bind
{
    class SymbolBind : public ContextObjectBind
    {
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<Symbol> cl(module.isolate());
            cl
                .inherit<ContextObject>()
                .property("dataType", &Symbol::getDataType, &Symbol::setDataType);
            module.class_("Symbol", cl);
        }
    };

    class VariableSymbolBind : public SymbolBind
    {
        static auto New(Context* ctx, const std::string& name, DataType* dataType) {
            return new VariableSymbol(ctx, nullptr, name, dataType);
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<VariableSymbol> cl(module.isolate());
            cl
                .inherit<Symbol>()
                .static_method("New", &New);
            module.class_("VariableSymbol", cl);
        }
    };

    class FunctionSymbolBind : public SymbolBind
    {
        static auto New(Context* ctx, const std::string& name, SignatureDataType* dataType) {
            auto stackSymbolTable = new StandartSymbolTable(ctx);
            auto instructionSymbolTable = new StandartSymbolTable(ctx);
            return new FunctionSymbol(ctx, nullptr, name, dataType, stackSymbolTable, instructionSymbolTable);
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<FunctionSymbol> cl(module.isolate());
            cl
                .inherit<Symbol>()
                .property("signature", &FunctionSymbol::getSignature)
                .property("stackSymbolTable", &FunctionSymbol::getStackSymbolTable)
                .property("instrSymbolTable", &FunctionSymbol::getInstructionSymbolTable)
                .static_method("New", &New);
            module.class_("FunctionSymbol", cl);
        }
    };

    class FunctionParameterSymbolBind : public SymbolBind
    {
        static auto New(Context* ctx, const std::string& name, DataType* dataType) {
            return new FunctionParameterSymbol(ctx, nullptr, name, dataType);
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<FunctionParameterSymbol> cl(module.isolate());
            cl
                .inherit<Symbol>()
                .static_method("New", &New);
            module.class_("FunctionParameterSymbol", cl);
        }
    };

    class StructureFieldSymbolBind : public SymbolBind
    {
        static auto New(Context* ctx, const std::string& name, DataType* dataType) {
            return new StructureFieldSymbol(ctx, nullptr, name, dataType);
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<StructureFieldSymbol> cl(module.isolate());
            cl
                .inherit<Symbol>()
                .static_method("New", &New);
            module.class_("StructureFieldSymbol", cl);
        }
    };
};