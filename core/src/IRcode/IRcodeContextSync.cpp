#include "SDA/Core/IRcode/IRcodeContextSync.h"
#include "SDA/Core/SymbolTable/StandartSymbolTable.h"
#include "SDA/Core/Utils/IOManip.h"

using namespace sda;
using namespace sda::ircode;

ContextSyncCallbacks::ContextSyncCallbacks(
    SymbolTable* globalSymbolTable,
    std::shared_ptr<CallingConvention> callingConvention)
    : m_globalSymbolTable(globalSymbolTable)
    , m_callingConvention(callingConvention)
{}

void ContextSyncCallbacks::onFunctionCreatedImpl(ircode::Function* function) {
    auto ctx = m_globalSymbolTable->getContext();
    auto offset = function->getEntryOffset();
    std::stringstream name;
    name << "function_" << utils::to_hex() << offset;

    auto signatureDt = new SignatureDataType(ctx, m_callingConvention);
    auto stackSymbolTable = new StandartSymbolTable(ctx);
    auto instrSymbolTable = new StandartSymbolTable(ctx);
    auto symbol = new FunctionSymbol(ctx, nullptr, name.str(), signatureDt, stackSymbolTable, instrSymbolTable);
    m_globalSymbolTable->addSymbol(offset, symbol);
}

void ContextSyncCallbacks::onFunctionRemovedImpl(ircode::Function* function) {
    auto offset = function->getEntryOffset();
    auto symbolInfo = m_globalSymbolTable->getSymbolAt(offset);
    m_globalSymbolTable->removeSymbol(offset);
    if (symbolInfo.symbol) {
        symbolInfo.symbol->destroy();
    }
}