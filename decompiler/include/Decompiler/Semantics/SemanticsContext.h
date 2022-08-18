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
    
    class SemanticsContextOperations : public std::set<SemanticsContextOperation> {
    public:
        using std::set<SemanticsContextOperation>::set;

        void join(const SemanticsContextOperations& other) {
            insert(other.begin(), other.end());
        }
    };
};