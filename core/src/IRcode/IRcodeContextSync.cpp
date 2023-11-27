#include "SDA/Core/IRcode/IRcodeContextSync.h"
#include "SDA/Core/IRcode/IRcodeHelper.h"
#include "SDA/Core/Researchers/ResearcherHelper.h"
#include "SDA/Core/SymbolTable/StandartSymbolTable.h"
#include "SDA/Core/Utils/String.h"
#include "SDA/Core/Commit.h"

using namespace sda;
using namespace sda::ircode;

ContextSync::SignatureToVariableMappingUpdater::SignatureToVariableMappingUpdater(
    Function* function,
    researcher::ResearcherPropagationContext* ctx,
    SignatureDataType* signatureDt
)
    : m_function(function)
    , m_ctx(ctx)
    , m_signatureDt(signatureDt)
{}

void ContextSync::SignatureToVariableMappingUpdater::start() {
    m_ctx->collect([&]() {
        update();
    });
    m_function->getProgram()->getEventPipe()->send(
        FunctionSignatureChangedEvent(m_function));
}

void ContextSync::SignatureToVariableMappingUpdater::update() {
    auto function = m_ctx->operation->getBlock()->getFunction();
    auto output = m_ctx->operation->getOutput();
    if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(m_ctx->operation)) {
        auto input = unaryOp->getInput();
        if (unaryOp->getId() == ircode::OperationId::LOAD) {
            updateForValue(input, CallingConvention::Storage::Read);
        }
    }
    if (!output->isUsed()) {
        auto outputAddrVal = output->getMemAddress().value;
        updateForValue(outputAddrVal, CallingConvention::Storage::Write);
    }
}

void ContextSync::SignatureToVariableMappingUpdater::updateForValue(
    std::shared_ptr<ircode::Value> value,
    CallingConvention::Storage::UseType type)
{
    if (auto reg = std::dynamic_pointer_cast<ircode::Register>(value)) {
        auto storageInfo = m_signatureDt->findStorageInfo({
            type,
            reg->getRegister().getRegId()
        });
        if (storageInfo) {
            updateForStorageInfo(storageInfo);
        }
    }
    else if (auto var = std::dynamic_pointer_cast<ircode::Variable>(value)) {
        auto linearExpr = GetLinearExpr(var, true);
        auto offset = linearExpr.getConstTermValue();
        auto platform = m_signatureDt->getContext()->getPlatform();
        auto baseTerms = ToBaseTerms(linearExpr, platform);
        for (auto& term : baseTerms) {
            if (auto baseRegister = ExtractRegister(term)) {
                auto regId = baseRegister->getRegId();
                if (regId == sda::Register::InstructionPointerId || regId == sda::Register::StackPointerId) {
                    auto storageInfo = m_signatureDt->findStorageInfo({
                        type,
                        regId,
                        offset
                    });
                    if (storageInfo) {
                        updateForStorageInfo(storageInfo);
                    }
                }
                break;
            }
        }
    }
}

void ContextSync::SignatureToVariableMappingUpdater::updateForStorageInfo(const CallingConvention::StorageInfo* storageInfo) {
    auto output = m_ctx->operation->getOutput();
    auto function = m_ctx->operation->getBlock()->getFunction();
    if (storageInfo->type == CallingConvention::StorageInfo::Return) {
        function->m_returnVar = output;
    }
    else {
        m_function->m_paramVars[storageInfo->paramIdx] = output;
    }
}

ContextSync::ContextSync(
    Program* program,
    SymbolTable* globalSymbolTable,
    std::shared_ptr<CallingConvention> callingConvention)
    : m_program(program)
    , m_globalSymbolTable(globalSymbolTable)
    , m_callingConvention(callingConvention)
{}

void ContextSync::handleFunctionCreated(const FunctionCreatedEvent& event) {
    auto ctx = m_globalSymbolTable->getContext();
    auto offset = event.function->getEntryOffset();
    std::stringstream name;
    name << "function_" << utils::ToHex(offset);

    auto signatureDt = new SignatureDataType(ctx, m_callingConvention, nullptr, name.str() + "-sig");
    auto stackSymbolTable = new StandartSymbolTable(ctx, nullptr, name.str() + "-stack-table");
    auto instrSymbolTable = new StandartSymbolTable(ctx, nullptr, name.str() + "-instr-table");
    auto symbol = new FunctionSymbol(ctx, nullptr, name.str(), signatureDt, stackSymbolTable, instrSymbolTable);
    m_globalSymbolTable->addSymbol(offset, symbol);
}

void ContextSync::handleFunctionRemoved(const FunctionRemovedEvent& event) {
    auto offset = event.function->getEntryOffset();
    auto symbolInfo = m_globalSymbolTable->getSymbolAt(offset);
    m_globalSymbolTable->removeSymbol(offset);
    if (symbolInfo.symbol) {
        symbolInfo.symbol->destroy();
    }
}

void ContextSync::handleFunctionDecompiled(const ircode::FunctionDecompiledEvent& event) {
    researcher::ResearcherPropagationContext ctx;
    for (auto block : event.blocks) { 
        for (auto& op : block->getOperations()) {
            ctx.addNextOperation(op.get());
        }
    }
    auto function = event.function;
    auto signatureDt = function->getFunctionSymbol()->getSignature();
    SignatureToVariableMappingUpdater updater(function, &ctx, signatureDt);
    updater.start();
}

void ContextSync::handleOperationRemoved(const ircode::OperationRemovedEvent& event) {
    auto function = event.op->getBlock()->getFunction();
    auto output = event.op->getOutput();
    if (output == function->m_returnVar) {
        function->m_returnVar = nullptr;
    } else {
        for (auto& paramVar : function->m_paramVars) {
            if (output == paramVar) {
                paramVar = nullptr;
                break;
            }
        }
    }
}

void ContextSync::handleObjectModified(const ObjectModifiedEvent& event) {
    if (event.state == Object::ModState::Before)
        return;
    // if function signature data type was modified
    if (auto signatureDt = dynamic_cast<SignatureDataType*>(event.object)) {
        for (auto funcSymbol : signatureDt->getFunctionSymbols()) {
            if (auto symbolTable = funcSymbol->getSymbolTable()) {
                if (symbolTable == m_globalSymbolTable) {
                    auto function = m_program->getFunctionAt(funcSymbol->getOffset());
                    {
                        // update 'signature->variable' mapping
                        auto oldParamVars = function->getParamVariables();
                        auto oldReturnVar = function->getReturnVariable();
                        function->m_paramVars.clear();
                        function->m_paramVars.resize(signatureDt->getParameters().size());
                        researcher::ResearcherPropagationContext ctx;
                        for (auto& [_, block] : function->getBlocks()) { 
                            for (auto& op : block.getOperations()) {
                                ctx.addNextOperation(op.get());
                            }
                        }
                        SignatureToVariableMappingUpdater updater(function, &ctx, signatureDt);
                        updater.start();
                    }
                    {
                        // update blocks
                        CommitScope commitScope(m_program->getEventPipe());
                        auto callOps = m_program->getCallsRefToFunction(function);
                        for (auto callOp : callOps) {
                            m_program->getEventPipe()->send(pcode::BlockUpdatedEvent(callOp->getBlock()->getPcodeBlock()));
                        }
                    }
                }
            }
        }
    }
}

std::shared_ptr<EventPipe> ContextSync::getEventPipe() {
    auto pipe = EventPipe::New("ContextSync");
    pipe->subscribeMethod(this, &ContextSync::handleFunctionCreated);
    pipe->subscribeMethod(this, &ContextSync::handleFunctionRemoved);
    pipe->subscribeMethod(this, &ContextSync::handleFunctionDecompiled);
    pipe->subscribeMethod(this, &ContextSync::handleOperationRemoved);
    auto contextPipe = pipe->connect(Context::CreateOptimizedEventPipe());
    contextPipe->subscribeMethod(this, &ContextSync::handleObjectModified);
    return pipe;
}
