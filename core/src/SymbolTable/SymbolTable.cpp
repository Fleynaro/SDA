#include "SDA/Core/SymbolTable/SymbolTable.h"
#include "SDA/Core/DataType/StructureDataType.h"

using namespace sda;

SymbolTable::SymbolTable(Context* context, Object::Id* id, const std::string& name)
    : ContextObject(context, id, name)
{}

std::list<SymbolTable::SymbolInfo> SymbolTable::getAllSymbolsRecursivelyAt(Offset offset, bool includeEmptySymbol) {
    std::list<SymbolInfo> result;
    auto symbolInfo = getSymbolAt(offset);
    while (symbolInfo.symbol) {
        result.push_back(symbolInfo);
        if (auto structDt = dynamic_cast<StructureDataType*>(symbolInfo.symbol->getDataType())) {
            symbolInfo = structDt->getSymbolTable()->getSymbolAt(symbolInfo.requestedOffset - symbolInfo.symbol->getOffset());
        } else {
            return result;
        }
    }
    if (includeEmptySymbol) {
        result.push_back(symbolInfo);
    }
    return result;
}

void SymbolTable::serialize(boost::json::object& data) const {
    ContextObject::serialize(data);
    data["class"] = Class;
}

void SymbolTable::destroy() {
    m_context->getSymbolTables()->remove(this);
}