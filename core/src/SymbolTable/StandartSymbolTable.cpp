#include "Core/SymbolTable/StandartSymbolTable.h"
#include "Core/Symbol/Symbol.h"
#include "Core/DataType/DataType.h"

using namespace sda;

StandartSymbolTable::StandartSymbolTable(Context* context, Object::Id* id, const std::string& name)
    : SymbolTable(context, id, name)
{}

size_t StandartSymbolTable::getUsedSize() const {
    if (m_symbols.empty())
        return 0;
    const auto& [symbolOffset, symbol] = *m_symbols.rbegin();
    return symbolOffset + symbol->getDataType()->getSize();
}

void StandartSymbolTable::addSymbol(Offset offset, Symbol* symbol) {
    if (getSymbolAt(offset).symbol)
        throw std::runtime_error("Symbol already exists at offset " + std::to_string(offset));
    notifyModified(Object::ModState::Before);
    m_symbols[offset] = symbol;
    notifyModified(Object::ModState::After);
}

void StandartSymbolTable::removeSymbol(Offset offset) {
    notifyModified(Object::ModState::Before);
    auto [_, symbolOffset, symbol] = getSymbolAt(offset);
    if (symbol) {
        m_symbols.erase(symbolOffset);
    }
    notifyModified(Object::ModState::After);
}

std::list<SymbolTable::SymbolInfo> StandartSymbolTable::getAllSymbols() {
    std::list<SymbolInfo> result;
    for (const auto& [symbolOffset, symbol] : m_symbols) {
        result.push_back({ this, symbolOffset, symbol });
    }
    return result;
}

SymbolTable::SymbolInfo StandartSymbolTable::getSymbolAt(Offset offset) {
    if (!m_symbols.empty()) {
        const auto it = std::prev(m_symbols.upper_bound(offset));
        if (it != m_symbols.end()) {
            const auto& [symbolOffset, symbol] = *it;
            if (offset < symbolOffset + symbol->getDataType()->getSize()) {
                return {
                    this,
                    symbolOffset,
                    symbol
                };
            }
        }
    }
    return SymbolInfo();
}

const std::map<Offset, Symbol*>& StandartSymbolTable::getSymbolsMap() const {
    return m_symbols;
}

void StandartSymbolTable::setSymbols(const std::map<Offset, Symbol*>& symbols) {
    notifyModified(Object::ModState::Before);
    m_symbols = symbols;
}

void StandartSymbolTable::serialize(boost::json::object& data) const {
    SymbolTable::serialize(data);
    data["type"] = Type;

    // serialize all symbols
    boost::json::array symbols;
    for (const auto& [offset, symbol] : m_symbols) {
        boost::json::object symbolData;
        symbolData["offset"] = offset;
        symbolData["symbol"] = symbol->serializeId();
        symbols.push_back(symbolData);
    }
    data["symbols"] = symbols;
}

void StandartSymbolTable::deserialize(boost::json::object& data) {
    SymbolTable::deserialize(data);

    // deserialize all symbols
    m_symbols.clear();
    const auto& symbols = data["symbols"].get_array();
    for (auto symbolData : symbols) {
        auto offset = symbolData.get_object()["offset"].get_uint64();
        auto symbol = m_context->getSymbols()->get(symbolData.get_object()["symbol"]);
        m_symbols[offset] = symbol;
    }
}