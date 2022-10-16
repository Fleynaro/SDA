#include "SDA/Core/SymbolTable/SymbolTable.h"
#include "SDA/Core/DataType/StructureDataType.h"

using namespace sda;

SymbolTable::SymbolTable(Context* context, Object::Id* id, const std::string& name)
    : ContextObject(context, id, name)
{}

std::list<SymbolTable::SymbolInfo> SymbolTable::getAllSymbolsRecursivelyAt(Offset offset) {
    std::list<SymbolInfo> result;
    auto symbolInfo = getSymbolAt(offset);
    auto symbol = symbolInfo.symbol;
    while (symbol) {
        result.push_back(symbolInfo);
        if (auto structDt = dynamic_cast<StructureDataType*>(symbol->getDataType())) {
            offset -= symbolInfo.symbolOffset;
            symbolInfo = getSymbolAt(offset);
            symbol = symbolInfo.symbol;
        } else {
            symbol = nullptr;
        }
    }
    return result;
}

void SymbolTable::serialize(boost::json::object& data) const {
    ContextObject::serialize(data);
    data["collection"] = Collection;
}

void SymbolTable::destroy() {
    m_context->getSymbolTables()->remove(this);
}