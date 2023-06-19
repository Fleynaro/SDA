#pragma once
#include "IRcodeProgram.h"
#include "IRcodeEvents.h"
#include "SDA/Core/Platform/CallingConvention.h"
#include "SDA/Core/Semantics/Semantics.h"

namespace sda::ircode
{
    class ContextSync
    {
        class SignatureToVariableMappingUpdater {
            semantics::SemanticsPropagationContext* m_ctx;
            SignatureDataType* m_signatureDt;
        public:
            SignatureToVariableMappingUpdater(
                semantics::SemanticsPropagationContext* ctx,
                SignatureDataType* signatureDt
            );

            void start();
        private:
            void update();

            void updateForValue(std::shared_ptr<ircode::Value> value, CallingConvention::Storage::UseType type);

            void updateForStorageInfo(const CallingConvention::StorageInfo* storageInfo);
        };
        Program* m_program;
        SymbolTable* m_globalSymbolTable;
        std::shared_ptr<CallingConvention> m_callingConvention;

        void handleFunctionCreated(const FunctionCreatedEvent& event);

        void handleFunctionRemoved(const FunctionRemovedEvent& event);

        void handleFunctionDecompiled(const ircode::FunctionDecompiledEvent& event);

        void handleOperationRemoved(const ircode::OperationRemovedEvent& event);

        void handleObjectModified(const ObjectModifiedEvent& event);
    public:
        ContextSync(
            Program* program,
            SymbolTable* globalSymbolTable,
            std::shared_ptr<CallingConvention> callingConvention);

        std::shared_ptr<EventPipe> getEventPipe();
    };
};