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

    auto signatureDt = new SignatureDataType(ctx, m_callingConvention);
    auto stackSymbolTable = new StandartSymbolTable(ctx);
    auto instrSymbolTable = new StandartSymbolTable(ctx);
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
    pipe->handleMethod(this, &ContextSync::handleFunctionCreated);
    pipe->handleMethod(this, &ContextSync::handleFunctionRemoved);
    return pipe;
}
