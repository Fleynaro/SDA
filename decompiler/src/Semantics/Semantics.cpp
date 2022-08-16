#include "Decompiler/Semantics/Semantics.h"
#include "Decompiler/Semantics/SemanticsManager.h"
#include "Core/DataType/StructureDataType.h"

using namespace sda;
using namespace sda::decompiler;

Semantics::Semantics(const std::shared_ptr<SourceInfo>& sourceInfo, const MetaInfo& metaInfo)
    : m_sourceInfo(sourceInfo), m_metaInfo(metaInfo)
{
    if (!m_sourceInfo->sourceSemantics)
        m_sourceInfo->sourceSemantics = this;
}

const std::shared_ptr<Semantics::SourceInfo>& Semantics::getSourceInfo() const {
    return m_sourceInfo;
}

const Semantics::MetaInfo& Semantics::getMetaInfo() const {
    return m_metaInfo;
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

DataTypeSemantics::DataTypeSemantics(
    const std::shared_ptr<SourceInfo>& sourceInfo,
    DataType* dataType,
    const SliceInfo& sliceInfo = {},
    const MetaInfo& metaInfo = {})
    : Semantics(sourceInfo, metaInfo)
    , m_dataType(dataType)
    , m_sliceInfo(sliceInfo)
{}

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

std::unique_ptr<Semantics> DataTypeSemantics::clone(const MetaInfo& metaInfo) const {
    return std::make_unique<DataTypeSemantics>(getSliceInfo(), m_dataType, m_sliceInfo, metaInfo);
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

SymbolTableSemantics::SymbolTableSemantics(
    const std::shared_ptr<SourceInfo>& sourceInfo,
    SymbolTable* symbolTable,
    const MetaInfo& metaInfo = {})
    : Semantics(sourceInfo, metaInfo)
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

std::unique_ptr<Semantics> SymbolTableSemantics::clone(const MetaInfo& metaInfo) const {
    return std::make_unique<SymbolTableSemantics>(getSourceInfo(), m_symbolTable, metaInfo);
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