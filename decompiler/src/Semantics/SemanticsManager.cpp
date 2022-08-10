#include "Decompiler/Semantics/SemanticsManager.h"

using namespace sda;
using namespace sda::decompiler;

void SemanticsManager::SemanticsHolder::add(const Semantics* sem, bool emit) {
    holdedSemantics.insert(sem);
    if (emit)
        emitedSemantics.insert(sem);
}

bool SemanticsManager::SemanticsHolder::has(const Semantics* sem, bool emit) const {
    return emit ? emitedSemantics.find(sem) != emitedSemantics.end()
                : holdedSemantics.find(sem) != holdedSemantics.end();
}

const Semantics* SemanticsManager::SemanticsHolder::find(const std::string& name, bool emit) const {
    auto& semantics = emit ? emitedSemantics : holdedSemantics;
    for (auto& sem : semantics) {
        if (sem->getName() == name)
            return sem;
    }
    return nullptr;
}

SemanticsManager::SemanticsManager(Context* context)
    : m_context(context)
{}

Context* SemanticsManager::getContext() const {
    return m_context;
}

Semantics* SemanticsManager::addSemantics(std::unique_ptr<Semantics> semantics) {
    auto pSem = semantics.get();
    m_semanticsList.push_back(std::move(semantics));
    return pSem;
}

SemanticsManager::SemanticsHolder* SemanticsManager::getHolder(Symbol* symbol) {
    auto it = m_symbolSemanticsHolders.find(symbol);
    if (it != m_symbolSemanticsHolders.end())
        return &it->second;
    m_symbolSemanticsHolders[symbol] = SemanticsHolder();
    return &m_symbolSemanticsHolders[symbol];
}

SemanticsManager::SemanticsHolder* SemanticsManager::getHolder(DataType* dataType) {
    auto it = m_dataTypeSemanticsHolders.find(dataType);
    if (it != m_dataTypeSemanticsHolders.end())
        return &it->second;
    m_dataTypeSemanticsHolders[dataType] = SemanticsHolder();
    return &m_dataTypeSemanticsHolders[dataType];
}

SemanticsManager::SemanticsHolder* SemanticsManager::getHolder(std::shared_ptr<ircode::Variable> variable) {
    auto it = m_variableSemanticsHolders.find(variable.get());
    if (it != m_variableSemanticsHolders.end())
        return &it->second;
    m_variableSemanticsHolders[variable.get()] = SemanticsHolder();
    return &m_variableSemanticsHolders[variable.get()];
}

void SemanticsManager::addSymbol(Symbol* symbol, std::shared_ptr<ircode::Variable> variable) {
    auto& vars = getVariablesForSymbol(symbol);
    vars.push_back(variable.get());
}

void SemanticsManager::addSymbolOffset(SymbolTable* symbolTable, Offset offset, std::shared_ptr<ircode::Variable> variable) {
    auto& vars = getVariablesForSymbolOffset(symbolTable, offset);
    vars.push_back(variable.get());
}

void SemanticsManager::addFuncReturn(SignatureDataType* signatureDt, std::shared_ptr<ircode::Variable> variable) {
    auto& vars = getVariablesForFuncReturn(signatureDt);
    vars.push_back(variable.get());
}

std::list<ircode::Variable*>& SemanticsManager::getVariablesForSymbol(Symbol* symbol) {
     if (m_symbolsToVariables.find(symbol) == m_symbolsToVariables.end())
        m_symbolsToVariables[symbol] = std::list<ircode::Variable*>();
    return m_symbolsToVariables[symbol];
}

std::list<ircode::Variable*>& SemanticsManager::getVariablesForSymbolOffset(SymbolTable* symbolTable, Offset offset) {
    if (m_symbolOffsetsToVariables.find(symbolTable) == m_symbolOffsetsToVariables.end())
        m_symbolOffsetsToVariables[symbolTable] = std::map<Offset, std::list<ircode::Variable*>>();
    auto& offsetToVariables = m_symbolOffsetsToVariables[symbolTable];
    if (offsetToVariables.find(offset) == offsetToVariables.end())
        offsetToVariables[offset] = std::list<ircode::Variable*>();
    return offsetToVariables[offset];
}

std::list<ircode::Variable*>& SemanticsManager::getVariablesForFuncReturn(SignatureDataType* signatureDt) {
    if (m_funcReturnsToVariables.find(signatureDt) == m_funcReturnsToVariables.end())
        m_funcReturnsToVariables[signatureDt] = std::list<ircode::Variable*>();
    return m_funcReturnsToVariables[signatureDt];
}