#include "Core/SymbolTable/StandartSymbolTable.h"
#include "Core/Symbol/Symbol.h"

using namespace sda;

StandartSymbolTable::StandartSymbolTable(Context* context, Object::Id* id, const std::string& name)
    : SymbolTable(context, id, name)
{}

void StandartSymbolTable::addSymbol(Offset offset, Symbol* symbol) {
    m_context->getCallbacks()->onObjectModified(this);
    m_symbols[offset] = symbol;
}

void StandartSymbolTable::removeSymbol(Offset offset) {
    m_context->getCallbacks()->onObjectModified(this);
    m_symbols.erase(offset);
}

Symbol* StandartSymbolTable::getSymbolAt(Offset offset) {
    auto it = m_symbols.find(offset);
    if (it == m_symbols.end())
        return nullptr;
    return it->second;
}

const std::map<Offset, Symbol*>& StandartSymbolTable::getSymbols() const {
    return m_symbols;
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
    }
    data["symbols"] = symbols;
}

void StandartSymbolTable::deserialize(boost::json::object& data) {
    SymbolTable::deserialize(data);

    // deserialize all symbols
    m_symbols.clear();
    const auto& symbols = data["symbol"].get_array();
    for (auto symbolData : symbols) {
        auto offset = symbolData.get_object()["offset"].get_uint64();
        auto symbol = m_context->getSymbols()->get(symbolData.get_object()["symbol"]);
        m_symbols[offset] = symbol;
    }
}