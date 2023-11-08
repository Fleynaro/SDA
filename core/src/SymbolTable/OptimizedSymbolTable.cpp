#include "SDA/Core/SymbolTable/OptimizedSymbolTable.h"
#include "SDA/Core/SymbolTable/StandartSymbolTable.h"
#include "SDA/Core/Utils/String.h"

using namespace sda;

OptimizedSymbolTable::OptimizedSymbolTable(
    Context* context,
    Object::Id* id,
    const std::string& name,
    Offset minOffset,
    Offset maxOffset,
    size_t fragmentsCount)
    : SymbolTable(context, id, name),
    m_minOffset(minOffset),
    m_maxOffset(maxOffset)
{
    m_context->getSymbolTables()->add(std::unique_ptr<OptimizedSymbolTable>(this));
    if (fragmentsCount > 0) {
        setFragmentsCount(fragmentsCount);
    }
}

void OptimizedSymbolTable::setFragmentsCount(size_t count) {
    notifyModified(Object::ModState::Before);

    // remove old symbol tables and save all symbols temporarily
    std::list<std::map<Offset, Symbol*>> allSymbols;
    for (auto symbolTable : m_symbolTables) {
        allSymbols.push_back(symbolTable->getSymbolsMap());
        symbolTable->destroy();
    }
    m_symbolTables.clear();

    // create new symbol tables
    for (size_t i = 0; i < count; i++) {
        auto symbolTable = new StandartSymbolTable(m_context, nullptr, getName() + ":" + std::to_string(i));
        m_symbolTables.push_back(symbolTable);
    }

    // add all saved symbols to new symbol tables
    for (const auto& symbols : allSymbols) {
        for (const auto& [offset, symbol] : symbols) {
            addSymbol(offset, symbol);
        }
    }

    notifyModified(Object::ModState::After);
}

const std::vector<StandartSymbolTable*>& OptimizedSymbolTable::getSymbolTables() const {
    return m_symbolTables;
}

size_t OptimizedSymbolTable::getUsedSize() const {
    for (auto it = m_symbolTables.rbegin(); it != m_symbolTables.rend(); it++) {
        if (auto usedSize = (*it)->getUsedSize())
            return usedSize;
    }
    return 0;
}

void OptimizedSymbolTable::addSymbol(Offset offset, Symbol* symbol) {
    auto symbolInfo = getSymbolAt(offset);
    if (symbolInfo.symbol) {
        auto message = "Symbol already exists at offset 0x" + utils::ToHex(symbolInfo.symbolOffset);
        throw std::runtime_error(message);
    }
    auto symbolTable = selectSymbolTable(offset);
    symbolTable->addSymbol(offset, symbol);
}

void OptimizedSymbolTable::removeSymbol(Offset offset) {
    auto symbolInfo = getSymbolAt(offset);
    if (symbolInfo.symbol)
        symbolInfo.symbolTable->removeSymbol(offset);
    getContext()->getEventPipe()->send(SymbolTableSymbolRemovedEvent(this, offset));
}

std::list<SymbolTable::SymbolInfo> OptimizedSymbolTable::getAllSymbols() {
    std::list<SymbolInfo> result;
    for (auto symbolTable : m_symbolTables) {
        auto symbols = symbolTable->getAllSymbols();
        result.insert(result.end(), symbols.begin(), symbols.end());
    }
    return result;
}

SymbolTable::SymbolInfo OptimizedSymbolTable::getSymbolAt(Offset offset) {
    for (auto symbolTable : m_symbolTables) {
        auto symbolInfo = symbolTable->getSymbolAt(offset);
        if (symbolInfo.symbol)
            return symbolInfo;
    }
    return SymbolInfo();
}

void OptimizedSymbolTable::serialize(boost::json::object& data) const {
    SymbolTable::serialize(data);
    data["type"] = Type;
    data["min_offset"] = m_minOffset;
    data["max_offset"] = m_maxOffset;

    // serialize all symbol tables
    boost::json::array symbolTables;
    for (auto symbolTable : m_symbolTables) {
        symbolTables.push_back(symbolTable->serializeId());
    }
    data["symbol_tables"] = symbolTables;
}

void OptimizedSymbolTable::deserialize(boost::json::object& data) {
    SymbolTable::deserialize(data);
    m_minOffset = utils::get_number<Offset>(data["min_offset"]);
    m_maxOffset = utils::get_number<Offset>(data["max_offset"]);

    // deserialize all symbol tables
    m_symbolTables.clear();
    const auto& symbolTables = data["symbol_tables"].get_array();
    for (auto symbolTableId : symbolTables) {
        auto symbolTable = m_context->getSymbolTables()->get(symbolTableId);
        if (auto stdSymbolTable = dynamic_cast<StandartSymbolTable*>(symbolTable)) {
            m_symbolTables.push_back(stdSymbolTable);
        }
    }
}

void OptimizedSymbolTable::destroy() {
    for (auto symbolTable : m_symbolTables) {
        symbolTable->destroy();
    }
    SymbolTable::destroy();
}

StandartSymbolTable* OptimizedSymbolTable::selectSymbolTable(Offset offset) const {
    auto fragmentSize = (m_maxOffset - m_minOffset) / m_symbolTables.size();
    auto tableSymbolIdx = (offset - m_minOffset) / fragmentSize;
    if (tableSymbolIdx < 0 || tableSymbolIdx >= m_symbolTables.size())
        throw std::runtime_error("Invalid offset");
    return m_symbolTables[tableSymbolIdx];
}