#include "Decompiler/Semantics/Semantics.h"
#include "Decompiler/Semantics/SemanticsManager.h"

using namespace sda;
using namespace sda::decompiler;

DataTypeSemantics::DataTypeSemantics(DataType* dataType, SymbolTable* symbolTable)
    : m_dataType(dataType)
    , m_symbolTable(symbolTable)
{}

const std::string& DataTypeSemantics::getName() const {
    static const std::string name = "DataTypeSemantics";
    return name;
}

DataType* DataTypeSemantics::getDataType() const {
    return m_dataType;
}

SymbolTable* DataTypeSemantics::getSymbolTable() const {
    return m_symbolTable;
}

Semantics::FilterFunction DataTypeSemantics::Filter(const DataTypeFilterFunction& filter) {
    return [filter](const Semantics* sem) {
        if (auto dataTypeSem = dynamic_cast<const DataTypeSemantics*>(sem))
            return filter(dataTypeSem);
        return false;
    };
}