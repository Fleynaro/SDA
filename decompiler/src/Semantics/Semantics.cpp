#include "Decompiler/Semantics/Semantics.h"
#include "Decompiler/Semantics/SemanticsManager.h"

using namespace sda;
using namespace sda::decompiler;

Semantics::Semantics(SemanticsObject* sourceObject)
    : m_sourceObject(sourceObject)
{
    m_sourceObject->m_semantics.insert(this);
}

SemanticsObject* Semantics::getSourceObject() const {
    return m_sourceObject;
}

bool Semantics::isSimiliarTo(const Semantics* other) const {
    return false;
}

DataTypeSemantics::DataTypeSemantics(SemanticsObject* sourceObject, DataType* dataType, SymbolTable* symbolTable)
    : Semantics(sourceObject)
    , m_dataType(dataType)
    , m_symbolTable(symbolTable)
{}

const std::string& DataTypeSemantics::getName() const {
    return "DataTypeSemantics";
}

DataType* DataTypeSemantics::getDataType() const {
    return m_dataType;
}

SymbolTable* DataTypeSemantics::getSymbolTable() const {
    return m_symbolTable;
}

bool DataTypeSemantics::isSimiliarTo(const Semantics* other) const {
    return Filter(m_dataType)(other);
}

DataTypeSemantics::FilterFunction DataTypeSemantics::Filter(const DataType* dataType) {
    DataTypeSemantics::DataTypeFilterFunction filter;
    if (dataType->isScalar(ScalarType::SignedInt)) {
        filter = [](const DataTypeSemantics* sem) {
            return sem->getDataType()->isScalar(ScalarType::SignedInt);
        };
    } else if (dataType->isScalar(ScalarType::FloatingPoint)) {
        filter = [](const DataTypeSemantics* sem) {
            return sem->getDataType()->isScalar(ScalarType::FloatingPoint);
        };
    } else {
        filter = [dataType](const DataTypeSemantics* sem) {
            return sem->getDataType()->getName() == dataType->getName();
        };
    }
    return Filter(filter);
}

Semantics::FilterFunction DataTypeSemantics::Filter(const DataTypeFilterFunction& filter) {
    return [filter](const Semantics* sem) {
        if (auto dataTypeSem = dynamic_cast<const DataTypeSemantics*>(sem))
            return filter(dataTypeSem);
        return false;
    };
}