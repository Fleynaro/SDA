#pragma once
#include "SDA/Core/Symbol/VariableSymbol.h"
#include "SDA/Core/Symbol/FunctionSymbol.h"
#include "SDA/Core/Symbol/FunctionParameterSymbol.h"
#include "SDA/Core/Symbol/StructureFieldSymbol.h"

namespace sda::bind
{
    class SymbolBind : public ContextObjectBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<Symbol>(module);
            cl
                .inherit<ContextObject>()
                .property("dataType", &Symbol::getDataType, &Symbol::setDataType);
            RegisterClassName(cl, "Symbol");
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
            auto cl = NewClass<VariableSymbol>(module);
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
            auto cl = NewClass<FunctionSymbol>(module);
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
            auto cl = NewClass<FunctionParameterSymbol>(module);
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
            auto cl = NewClass<StructureFieldSymbol>(module);
            cl
                .inherit<Symbol>()
                .static_method("New", &New);
            module.class_("StructureFieldSymbol", cl);
        }
    };
};