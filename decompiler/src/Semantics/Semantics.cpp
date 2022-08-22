#include "Decompiler/Semantics/Semantics.h"
#include "Decompiler/Semantics/SemanticsManager.h"
#include "Core/DataType/StructureDataType.h"

using namespace sda;
using namespace sda::decompiler;

Semantics::Semantics(
    SemanticsObject* holder,
    const std::shared_ptr<SourceInfo>& sourceInfo,
    const MetaInfo& metaInfo)
    : m_holder(holder)
    , m_sourceInfo(sourceInfo)
    , m_metaInfo(metaInfo)
{
    m_holder->m_semantics.push_back(std::unique_ptr<Semantics>(this));
    if (!m_sourceInfo->sourceSemantics)
        m_sourceInfo->sourceSemantics = this;
}

Semantics::~Semantics() {
    m_holder->m_semantics.remove_if([this](const std::unique_ptr<Semantics>& sem) {
        return sem.get() == this;
    });
    for (auto successor : m_successors)
        successor->m_predecessors.remove(this);
    for (auto predecessor : m_predecessors)
        predecessor->m_successors.remove(this);
}

SemanticsObject* Semantics::getHolder() const {
    return m_holder;
}

const std::shared_ptr<Semantics::SourceInfo>& Semantics::getSourceInfo() const {
    return m_sourceInfo;
}

bool Semantics::isSource(CreatorType creatorType) const {
    return m_sourceInfo->sourceSemantics == this &&
            m_sourceInfo->creatorType == creatorType;
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

Semantics::FilterFunction Semantics::FilterSource(const std::shared_ptr<SourceInfo>& source) {
    return [source](const Semantics* sem) {
        return source == sem->getSourceInfo();
    };
}

DataTypeSemantics::DataTypeSemantics(
    SemanticsObject* holder,
    const std::shared_ptr<SourceInfo>& sourceInfo,
    DataType* dataType,
    const SliceInfo& sliceInfo,
    const MetaInfo& metaInfo)
    : Semantics(holder, sourceInfo, metaInfo)
    , m_dataType(dataType)
    , m_sliceInfo(sliceInfo)
{}

std::string DataTypeSemantics::getName() const {
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

Semantics* DataTypeSemantics::clone(SemanticsObject* holder, const MetaInfo& metaInfo) const {
    return new DataTypeSemantics(holder, getSourceInfo(), m_dataType, m_sliceInfo, metaInfo);
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
    SemanticsObject* holder,
    const std::shared_ptr<SourceInfo>& sourceInfo,
    SymbolTable* symbolTable,
    const MetaInfo& metaInfo)
    : Semantics(holder, sourceInfo, metaInfo)
    , m_symbolTable(symbolTable)
{}

std::string SymbolTableSemantics::getName() const {
    return "SymbolTableSemantics";
}

bool SymbolTableSemantics::isSimiliarTo(const Semantics* other) const {
    return Filter(
        [=](const SymbolTableSemantics* sem) {
            return m_symbolTable == sem->getSymbolTable();
        }
    )(other);
}

Semantics* SymbolTableSemantics::clone(SemanticsObject* holder, const MetaInfo& metaInfo) const {
    return new SymbolTableSemantics(holder, getSourceInfo(), m_symbolTable, metaInfo);
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