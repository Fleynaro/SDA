#pragma once
#include "Semantics.h"
#include "SDA/Core/Commit.h"
#include "SDA/Core/DataType/SignatureDataType.h"
#include "SDA/Core/IRcode/IRcodeBlock.h"
#include "SDA/Core/IRcode/IRcodeEvents.h"

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
                return nullptr;
            }
            return &it->second;
        }

        void addSignatureStorageInfo(
            ircode::Function* function,
            const CallingConvention::StorageInfo& storageInfo,
            std::shared_ptr<ircode::Variable> variable)
        {
            auto it = m_funcToSignature.find(function);
            if (it == m_funcToSignature.end()) {
                m_funcToSignature[function] = Signature();
                it = m_funcToSignature.find(function);
            }
            auto& signature = it->second;
            auto& storageInfos = signature.storageInfos;
            auto it2 = std::find(storageInfos.begin(), storageInfos.end(), storageInfo);
            if (it2 == storageInfos.end()) {
                storageInfos.push_back({ storageInfo });
                it2 = std::prev(storageInfos.end());
            }
            if (std::find(it2->variables.begin(), it2->variables.end(), variable) == it2->variables.end()) {
                it2->variables.push_back(variable);
            }
        }
    };

    class SignatureCollector
    {
        ircode::Program* m_program;
        Platform* m_platform;
        SignatureRepository* m_signatureRepo;
        std::shared_ptr<CallingConvention> m_callingConvention;

        class IRcodeEventHandler
        {
            SignatureCollector* m_collector;

            void handleFunctionDecompiled(const ircode::FunctionDecompiledEvent& event) {
                SemanticsPropagationContext ctx;
                for (auto block : event.blocks) { 
                    for (auto& op : block->getOperations()) {
                        ctx.addNextOperation(op.get());
                    }
                }
                ctx.collect([&]() {
                    m_collector->collect(ctx);
                });
                if (event.function->getFunctionSymbol()) {
                    auto sig = m_collector->m_signatureRepo->getSignature(event.function);
                    if (sig) {
                        CommitScope commit(m_collector->m_program->getEventPipe());
                        sig->updateSignatureDataType(event.function->getFunctionSymbol()->getSignature());
                    }
                }
            }
        public:
            IRcodeEventHandler(SignatureCollector* collector) : m_collector(collector) {}

            std::shared_ptr<EventPipe> getEventPipe() {
                auto pipe = EventPipe::New();
                pipe->subscribeMethod(this, &IRcodeEventHandler::handleFunctionDecompiled);
                return pipe;
            }
        };
        IRcodeEventHandler m_ircodeEventHandler;
    public:
        SignatureCollector(
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
        {
            m_program->getEventPipe()->connect(m_ircodeEventHandler.getEventPipe());
        }

        void collect(SemanticsPropagationContext& ctx)
        {
            auto function = ctx.operation->getBlock()->getFunction();
            auto output = ctx.operation->getOutput();
            if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(ctx.operation)) {
                auto input = unaryOp->getInput();
                if (unaryOp->getId() == ircode::OperationId::LOAD) {
                    exploreValue(ctx, input, CallingConvention::Storage::Read);
                }
            }
            auto outputAddrVal = output->getMemAddress().value;
            exploreValue(ctx, outputAddrVal, CallingConvention::Storage::Write);
        }
    private:
        void exploreValue(
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
                auto linearExpr = var->getLinearExpr();
                Offset offset = linearExpr.getConstTermValue();
                for (auto& term : linearExpr.getTerms()) {
                    if (term.factor != 1 || term.value->getSize() != m_platform->getPointerSize())
                        continue;
                    if (auto baseRegister = extractRegister(term.value)) {
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

        const Register* extractRegister(std::shared_ptr<ircode::Value> value) {
            if (auto variable = std::dynamic_pointer_cast<ircode::Variable>(value)) {
                if (auto unarySrcOp = dynamic_cast<const ircode::UnaryOperation*>(variable->getSourceOperation())) {
                    if (unarySrcOp->getId() == ircode::OperationId::LOAD) {
                        if (auto reg = std::dynamic_pointer_cast<ircode::Register>(unarySrcOp->getInput())) {
                            return &reg->getRegister();
                        }
                    }
                }
            }
            return nullptr;
        }
    };
};