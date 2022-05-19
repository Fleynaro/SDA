#include "Core/Symbol/SymbolTable.h"
#include "Core/Symbol/Symbol.h"

using namespace sda;

SymbolTable::SymbolTable(Context* context, Object::Id* id, const std::string& name)
    : ContextObject(context, id, name)
{
    m_context->getSymbolTables()->add(std::unique_ptr<SymbolTable>(this));
}

void SymbolTable::addSymbol(Offset offset, Symbol* symbol) {
    m_context->getCallbacks()->onObjectModified(this);
    m_symbols[offset] = symbol;
}

Symbol* SymbolTable::getSymbolAt(Offset offset) const {
    auto it = m_symbols.find(offset);
    if (it == m_symbols.end()) {
        return nullptr;
    }
    return it->second;
}

void SymbolTable::serialize(boost::json::object& data) const {
    ContextObject::serialize(data);

    // serialize all symbols
    boost::json::array symbols;
    for (const auto& [offset, symbol] : m_symbols) {
        boost::json::object symbolData;
        symbolData["offset"] = offset;
        symbolData["symbol"] = symbol->serializeId();
    }
    data["symbols"] = symbols;
}

void SymbolTable::deserialize(boost::json::object& data) {
    ContextObject::deserialize(data);

    // deserialize all symbols
    const auto& symbols = data["symbol"].get_array();
    for (auto symbolData : symbols) {
        auto offset = symbolData.get_object()["offset"].get_uint64();
        auto symbol = m_context->getSymbols()->get(symbolData.get_object()["symbol"]);
        m_symbols[offset] = symbol;
    }
}

void SymbolTable::destroy() {
    m_context->getSymbolTables()->remove(this);
}