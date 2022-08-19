#pragma once
#include <set>
#include "Core/SymbolTable/SymbolTable.h"
#include "Core/Symbol/FunctionSymbol.h"
#include "Core/IRcode/IRcodeOperation.h"

namespace sda::decompiler
{
    struct SemanticsContext {
        SymbolTable* globalSymbolTable;
        FunctionSymbol* functionSymbol;
    };

    struct SemanticsContextOperation {
        std::shared_ptr<SemanticsContext> context;
        const ircode::Operation* operation;

        bool operator<(const SemanticsContextOperation& other) const {
            return operation < other.operation;
        }
    };

    using SemanticsContextOperations = std::set<SemanticsContextOperation>;
};