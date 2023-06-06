#pragma once
#include "IRcodeProgram.h"
#include "SDA/Core/Platform/CallingConvention.h"

namespace sda::ircode
{
    class ContextSyncCallbacks : public ircode::Program::Callbacks
    {
        SymbolTable* m_globalSymbolTable;
        std::shared_ptr<CallingConvention> m_callingConvention;

        void onFunctionCreatedImpl(ircode::Function* function) override;

        void onFunctionRemovedImpl(ircode::Function* function) override;
    public:
        ContextSyncCallbacks(
            SymbolTable* globalSymbolTable,
            std::shared_ptr<CallingConvention> callingConvention);
    };
};