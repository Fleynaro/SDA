#pragma once
#include "SDA/Core/SymbolTable/StandartSymbolTable.h"
#include "SDA/Core/SymbolTable/OptimizedSymbolTable.h"

namespace sda::bind
{
    class SymbolTableBind : public ContextObjectBind
    {
    public:
        static void Init(v8pp::module& module) {
            {
                auto cl = NewClass<SymbolTable::SymbolInfo>(module);
                cl
                    .auto_wrap_objects(true)
                    .var("symbolTable", &SymbolTable::SymbolInfo::symbolTable)
                    .var("symbolOffset", &SymbolTable::SymbolInfo::symbolOffset)
                    .var("symbol", &SymbolTable::SymbolInfo::symbol);
                module.class_("SymbolTable_SymbolInfo", cl);
            }

            {
                auto cl = NewClass<SymbolTable>(module);
                cl
                    .inherit<ContextObject>()
                    .property("usedSize", &SymbolTable::getUsedSize)
                    .method("addSymbol", &SymbolTable::addSymbol)
                    .method("removeSymbol", &SymbolTable::removeSymbol)
                    .method("getAllSymbols", &SymbolTable::getAllSymbols)
                    .method("getSymbolAt", &SymbolTable::getSymbolAt)
                    .method("getAllSymbolsRecursivelyAt", &SymbolTable::getAllSymbolsRecursivelyAt);
                module.class_("SymbolTable", cl);
            }
        }
    };

    class StandartSymbolTableBind : public SymbolTableBind
    {
        static auto New(Context* ctx, const std::string& name) {
            return new StandartSymbolTable(ctx, nullptr, name);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<StandartSymbolTable>(module);
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
            auto cl = NewClass<OptimizedSymbolTable>(module);
            cl
                .inherit<SymbolTable>()
                .property("symbolTables", &OptimizedSymbolTable::getSymbolTables)
                .method("setFragmentsCount", &OptimizedSymbolTable::setFragmentsCount)
                .static_method("New", &New);
            module.class_("OptimizedSymbolTable", cl);
        }
    };
};