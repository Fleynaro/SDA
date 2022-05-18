#include "Core/Symbol/SymbolTable.h"
#include "Core/Symbol/Symbol.h"

using namespace sda;

SymbolTable::SymbolTable(Context* context, ObjectId* id, const std::string& name)
    : ContextObject(context, id, name)
{
    m_context->getSymbolTables()->add(std::unique_ptr<SymbolTable>(this));
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
        symbols.push_back(symbol->serializeId());
    }
    data["symbols"] = symbols;
}

void SymbolTable::deserialize(boost::json::object& data) {
    ContextObject::deserialize(data);

    // deserialize all symbols
    const auto& symbolIds = data["symbols"].get_array();
    for (const auto& symbolId : symbolIds) {
        auto symbol = m_context->getSymbols()->get(symbolId);
        //m_symbols[symbol->getOffset()] = symbol;
    }
}

void SymbolTable::destroy() {
    m_context->getSymbolTables()->remove(getId());
}