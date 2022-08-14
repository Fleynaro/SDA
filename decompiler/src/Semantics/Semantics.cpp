#include "Decompiler/Semantics/Semantics.h"
#include "Decompiler/Semantics/SemanticsManager.h"

using namespace sda;
using namespace sda::decompiler;

Semantics::Semantics(SemanticsObject* sourceObject, size_t uncertaintyDegree)
    : m_sourceObject(sourceObject), m_uncertaintyDegree(uncertaintyDegree)
{
    m_sourceObject->addSemantics(this);
}

SemanticsObject* Semantics::getSourceObject() const {
    return m_sourceObject;
}

size_t Semantics::getUncertaintyDegree() const {
    return m_uncertaintyDegree;
}

void Semantics::addSuccessor(Semantics* sem) {
    m_successors.push_back(sem);
    sem->m_predecessors.push_back(this);
}

const std::list<Semantics*>& Semantics::getSuccessors() const {
    return m_successors;
}

const std::list<Semantics*>& Semantics::getPredecessors() const {
    return m_predecessors;
}

bool Semantics::isSimiliarTo(const Semantics* other) const {
    return false;
}

Semantics::FilterFunction Semantics::FilterAll() {
    return [](const Semantics*) {
        return true;
    };
}

Semantics::FilterFunction Semantics::FilterOr(const FilterFunction& filter1, const FilterFunction& filter2) {
    return [=](const Semantics* sem) {
        return filter1(sem) || filter2(sem);
    };
}

Semantics::FilterFunction Semantics::FilterAnd(const FilterFunction& filter1, const FilterFunction& filter2) {
    return [=](const Semantics* sem) {
        return filter1(sem) && filter2(sem);
    };
}

DataTypeSemantics::DataTypeSemantics(SemanticsObject* sourceObject, DataType* dataType, const SliceInfo& sliceInfo, size_t uncertaintyDegree)
    : Semantics(sourceObject, uncertaintyDegree)
    , m_dataType(dataType)
    , m_sliceInfo(sliceInfo)
{
    if (!m_sliceInfo.size)
        m_sliceInfo.size = m_dataType->getSize();
}

const std::string& DataTypeSemantics::getName() const {
    return "DataTypeSemantics";
}

DataType* DataTypeSemantics::getDataType() const {
    return m_dataType;
}

const DataTypeSemantics::SliceInfo& DataTypeSemantics::getSliceInfo() const {
    return m_sliceInfo;
}

bool DataTypeSemantics::isSimiliarTo(const Semantics* other) const {
    return Filter(m_dataType)(other);
}

std::unique_ptr<Semantics> DataTypeSemantics::clone(size_t uncertaintyDegree) const {
    return std::make_unique<DataTypeSemantics>(getSourceObject(), m_dataType, m_sliceInfo, uncertaintyDegree);
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

SymbolTableSemantics::SymbolTableSemantics(SemanticsObject* sourceObject, SymbolTable* symbolTable, size_t uncertaintyDegree)
    : Semantics(sourceObject, uncertaintyDegree)
    , m_symbolTable(symbolTable)
{}

const std::string& SymbolTableSemantics::getName() const {
    return "SymbolTableSemantics";
}

bool SymbolTableSemantics::isSimiliarTo(const Semantics* other) const {
    return Filter(
        [=](const SymbolTableSemantics* sem) {
            return m_symbolTable == sem->getSymbolTable();
        }
    )(other);
}

std::unique_ptr<Semantics> SymbolTableSemantics::clone(size_t uncertaintyDegree) const {
    return std::make_unique<SymbolTableSemantics>(getSourceObject(), m_symbolTable, uncertaintyDegree);
}

SymbolTable* SymbolTableSemantics::getSymbolTable() const {
    return m_symbolTable;
}

Semantics::FilterFunction SymbolTableSemantics::Filter(const SymbolTableFilterFunction& filter) {
    return [filter](const Semantics* sem) {
        if (auto symbolTableSem = dynamic_cast<const SymbolTableSemantics*>(sem))
            return filter(symbolTableSem);
        return false;
    };
}