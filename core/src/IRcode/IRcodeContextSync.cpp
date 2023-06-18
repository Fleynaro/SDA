#include "SDA/Core/IRcode/IRcodeContextSync.h"
#include "SDA/Core/SymbolTable/StandartSymbolTable.h"
#include "SDA/Core/Utils/IOManip.h"
#include "SDA/Core/Commit.h"

using namespace sda;
using namespace sda::ircode;

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
    name << "function_" << utils::to_hex() << offset;

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

void ContextSync::handleObjectModified(const ObjectModifiedEvent& event) {
    if (event.state == Object::ModState::Before)
        return;
    if (auto signatureDt = dynamic_cast<SignatureDataType*>(event.object)) {
        for (auto funcSymbol : signatureDt->getFunctionSymbols()) {
            if (auto symbolTable = funcSymbol->getSymbolTable()) {
                if (symbolTable == m_globalSymbolTable) {
                    auto function = m_program->getFunctionAt(funcSymbol->getOffset());
                    auto blocks = m_program->getBlocksRefToFunction(function);
                    CommitScope commitScope(m_program->getEventPipe());
                    for (auto block : blocks) {
                        m_program->getEventPipe()->send(pcode::BlockUpdatedEvent(block->getPcodeBlock()));
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
    auto contextPipe = pipe->connect(Context::CreateOptimizedEventPipe());
    contextPipe->subscribeMethod(this, &ContextSync::handleObjectModified);
    return pipe;
}
