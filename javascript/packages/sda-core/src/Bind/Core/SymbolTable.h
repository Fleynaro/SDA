#pragma once
#include "Core/SymbolTable/StandartSymbolTable.h"
#include "Core/SymbolTable/OptimizedSymbolTable.h"

namespace sda::bind
{
    class SymbolTableBind : public ContextObjectBind
    {
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<SymbolTable> cl(module.isolate());
            cl
                .inherit<ContextObject>()
                .property("usedSize", &SymbolTable::getUsedSize)
                .method("addSymbol", &SymbolTable::addSymbol)
                .method("removeSymbol", &SymbolTable::removeSymbol);
            module.class_("SymbolTable", cl);
        }
    };

    class StandartSymbolTableBind : public SymbolTableBind
    {
        static auto New(Context* ctx, const std::string& name) {
            return new StandartSymbolTable(ctx, nullptr, name);
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<StandartSymbolTable> cl(module.isolate());
            cl
                .inherit<SymbolTable>()
                .property("symbols", &StandartSymbolTable::getSymbolsMap, &StandartSymbolTable::setSymbols)
                .static_method("New", &New);
            module.class_("StandartSymbolTable", cl);
        }
    };

    class OptimizedSymbolTableBind : public SymbolTableBind
    {
        static auto New(
            Context* ctx,
            const std::string& name,
            Offset minOffset,
            Offset maxOffset,
            size_t fragmentsCount)
        {
            return new OptimizedSymbolTable(
                ctx,
                nullptr,
                name,
                minOffset,
                maxOffset,
                fragmentsCount);
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<OptimizedSymbolTable> cl(module.isolate());
            cl
                .inherit<SymbolTable>()
                .property("symbolTables", &OptimizedSymbolTable::getSymbolTables)
                .method("setFragmentsCount", &OptimizedSymbolTable::setFragmentsCount)
                .static_method("New", &New);
            module.class_("OptimizedSymbolTable", cl);
        }
    };
};