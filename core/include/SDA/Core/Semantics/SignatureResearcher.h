#pragma once
#include "Semantics.h"
#include "SDA/Core/Commit.h"
#include "SDA/Core/DataType/SignatureDataType.h"
#include "SDA/Core/IRcode/IRcodeBlock.h"
#include "SDA/Core/IRcode/IRcodeEvents.h"
#include "SDA/Core/IRcode/IRcodeHelper.h"

namespace sda::semantics
{
    struct Signature {
        struct StorageInfo : CallingConvention::StorageInfo {
            std::list<std::shared_ptr<ircode::Variable>> variables;
        };
        std::list<StorageInfo> storageInfos;

        ScalarDataType* getScalarDataType(Context* ctx, StorageInfo& storageInfo) {
            size_t size = 0;
            for (auto variable : storageInfo.variables) {
                size = std::max(size, variable->getSize());
            }
            auto dataType = ctx->getDataTypes()->getScalar(
                storageInfo.isStoringFloat ? ScalarType::FloatingPoint : ScalarType::UnsignedInt,
                size);
            return dataType;
        }

        void updateSignatureDataType(SignatureDataType* signatureDt) {
            auto ctx = signatureDt->getContext();
            signatureDt->clear();

            std::map<size_t, StorageInfo> paramIdxToStorageInfos;
            for (auto& storageInfo : storageInfos) {
                if (storageInfo.type == CallingConvention::StorageInfo::Parameter) {
                    paramIdxToStorageInfos[storageInfo.paramIdx] = storageInfo;
                }
            }

            std::vector<FunctionParameterSymbol*> parameters;
            size_t nextParamIdx = 0;
            for (auto& [paramIdx, storageInfo] : paramIdxToStorageInfos) {
                while (nextParamIdx < paramIdx) {
                    auto dataType = ctx->getDataTypes()->getScalar(ScalarType::UnsignedInt, 1);
                    auto paramName = std::string("unk_param") + std::to_string(nextParamIdx + 1);
                    parameters.push_back(new FunctionParameterSymbol(ctx, nullptr, paramName, dataType));
                    nextParamIdx++;
                }
                auto dataType = getScalarDataType(ctx, storageInfo);
                auto paramName = std::string("param") + std::to_string(paramIdx + 1);
                parameters.push_back(new FunctionParameterSymbol(ctx, nullptr, paramName, dataType));
                nextParamIdx = paramIdx + 1;
            }
            signatureDt->setParameters(parameters);

            for (auto& storageInfo : storageInfos) {
                if (storageInfo.type == CallingConvention::StorageInfo::Return) {
                    auto dataType = getScalarDataType(ctx, storageInfo);
                    signatureDt->setReturnType(dataType);
                    break;
                }
            }
        }
    };

    class SignatureRepository
    {
        std::map<ircode::Function*, Signature> m_funcToSignature;
    public:
        SignatureRepository()
        {}

        Signature* getSignature(ircode::Function* function) {
            auto it = m_funcToSignature.find(function);
            if (it == m_funcToSignature.end()) {
                m_funcToSignature[function] = Signature();
                it = m_funcToSignature.find(function);
            }
            return &it->second;
        }

        void addSignatureStorageInfo(
            ircode::Function* function,
            const CallingConvention::StorageInfo& storageInfo,
            std::shared_ptr<ircode::Variable> variable)
        {
            auto signature = getSignature(function);
            auto& storageInfos = signature->storageInfos;
            auto it2 = std::find(storageInfos.begin(), storageInfos.end(), storageInfo);
            if (it2 == storageInfos.end()) {
                storageInfos.push_back({ storageInfo });
                it2 = std::prev(storageInfos.end());
            }
            if (std::find(it2->variables.begin(), it2->variables.end(), variable) == it2->variables.end()) {
                it2->variables.push_back(variable);
            }
        }

        void removeVariable(ircode::Function* function, std::shared_ptr<ircode::Variable> variable) {
            auto signature = getSignature(function);
            for (auto it2 = signature->storageInfos.begin(); it2 != signature->storageInfos.end(); ) {
                auto& storageInfo = *it2;
                auto it3 = std::find(storageInfo.variables.begin(), storageInfo.variables.end(), variable);
                if (it3 != storageInfo.variables.end()) {
                    storageInfo.variables.erase(it3);
                    if (storageInfo.variables.empty()) {
                        it2 = signature->storageInfos.erase(it2);
                    }
                    else {
                        it2++;
                    }
                }
                else {
                    it2++;
                }
            }
        }
    };

    class SignatureResearcher
    {
        ircode::Program* m_program;
        Platform* m_platform;
        SignatureRepository* m_signatureRepo;
        std::shared_ptr<CallingConvention> m_callingConvention;

        class IRcodeEventHandler
        {
            SignatureResearcher* m_researcher;

            void handleFunctionDecompiled(const ircode::FunctionDecompiledEvent& event) {
                SemanticsPropagationContext ctx;
                for (auto block : event.blocks) { 
                    for (auto& op : block->getOperations()) {
                        ctx.addNextOperation(op.get());
                    }
                }
                ctx.collect([&]() {
                    m_researcher->research(ctx);
                });
                if (event.function->getFunctionSymbol()) {
                    CommitScope commit(m_researcher->m_program->getEventPipe());
                    auto sig = m_researcher->m_signatureRepo->getSignature(event.function);
                    sig->updateSignatureDataType(event.function->getFunctionSymbol()->getSignature());
                }
            }

            void handleOperationRemoved(const ircode::OperationRemovedEvent& event) {
                auto function = event.op->getBlock()->getFunction();
                auto output = event.op->getOutput();
                m_researcher->m_signatureRepo->removeVariable(function, output);
                // No need to call updateSignatureDataType here because the FunctionDecompiledEvent will be called anyway
            }
        public:
            IRcodeEventHandler(SignatureResearcher* researcher) : m_researcher(researcher) {}

            std::shared_ptr<EventPipe> getEventPipe() {
                auto pipe = EventPipe::New();
                pipe->subscribeMethod(this, &IRcodeEventHandler::handleFunctionDecompiled);
                pipe->subscribeMethod(this, &IRcodeEventHandler::handleOperationRemoved);
                return pipe;
            }
        };
        IRcodeEventHandler m_ircodeEventHandler;
    public:
        SignatureResearcher(
            ircode::Program* program,
            Platform* platform,
            std::shared_ptr<CallingConvention> callingConvention,
            SignatureRepository* signatureRepo
        )
            : m_program(program)
            , m_platform(platform)
            , m_signatureRepo(signatureRepo)
            , m_callingConvention(callingConvention)
            , m_ircodeEventHandler(this)
        {}

        std::shared_ptr<EventPipe> getEventPipe() {
            return m_ircodeEventHandler.getEventPipe();
        }

        void research(SemanticsPropagationContext& ctx)
        {
            auto function = ctx.operation->getBlock()->getFunction();
            auto output = ctx.operation->getOutput();
            if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(ctx.operation)) {
                auto input = unaryOp->getInput();
                if (unaryOp->getId() == ircode::OperationId::LOAD) {
                    researchValue(ctx, input, CallingConvention::Storage::Read);
                }
            }
            if (!output->isUsed()) {
                auto outputAddrVal = output->getMemAddress().value;
                researchValue(ctx, outputAddrVal, CallingConvention::Storage::Write);
            }
        }
    private:
        void researchValue(
            SemanticsPropagationContext& ctx,
            std::shared_ptr<ircode::Value> value,
            CallingConvention::Storage::UseType type)
        {
            auto function = ctx.operation->getBlock()->getFunction();
            auto output = ctx.operation->getOutput();
            if (auto reg = std::dynamic_pointer_cast<ircode::Register>(value)) {
                CallingConvention::Storage storage = {
                    type,
                    reg->getRegister().getRegId()
                };
                CallingConvention::StorageInfo storageInfo;
                if (m_callingConvention->getStorageInfo(storage, storageInfo)) {
                    m_signatureRepo->addSignatureStorageInfo(function, storageInfo, output);
                }
            }
            else if (auto var = std::dynamic_pointer_cast<ircode::Variable>(value)) {
                auto linearExpr = ircode::Value::GetLinearExpr(var, true);
                Offset offset = linearExpr.getConstTermValue();
                for (auto& term : linearExpr.getTerms()) {
                    if (term.factor != 1 || term.value->getSize() != m_platform->getPointerSize())
                        continue;
                    if (auto baseRegister = ExtractRegister(term.value)) {
                        CallingConvention::Storage storage = {
                            type,
                            baseRegister->getRegId(),
                            offset
                        };
                        CallingConvention::StorageInfo storageInfo;
                        if (m_callingConvention->getStorageInfo(storage, storageInfo)) {
                            m_signatureRepo->addSignatureStorageInfo(function, storageInfo, output);
                        }
                        break;
                    }
                }
            }
        }
    };
};