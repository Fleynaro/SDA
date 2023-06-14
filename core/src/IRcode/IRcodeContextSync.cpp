#include "SDA/Core/IRcode/IRcodeContextSync.h"
#include "SDA/Core/SymbolTable/StandartSymbolTable.h"
#include "SDA/Core/Utils/IOManip.h"

using namespace sda;
using namespace sda::ircode;

ContextSync::ContextSync(
    SymbolTable* globalSymbolTable,
    std::shared_ptr<CallingConvention> callingConvention)
    : m_globalSymbolTable(globalSymbolTable)
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

std::shared_ptr<EventPipe> ContextSync::getEventPipe() {
    auto pipe = EventPipe::New();
    pipe->subscribeMethod(this, &ContextSync::handleFunctionCreated);
    pipe->subscribeMethod(this, &ContextSync::handleFunctionRemoved);
    return pipe;
}
