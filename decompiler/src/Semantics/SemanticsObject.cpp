#include "Decompiler/Semantics/SemanticsObject.h"

using namespace sda;
using namespace sda::decompiler;

bool SemanticsObject::addSemantics(Semantics* sem, bool emit) {
    sem->m_holders.insert(this);
    if (emit) {
        m_semantics.insert(sem);
        return m_emittedSemantics.insert(sem).second;
    }
    return m_semantics.insert(sem).second;
}

bool SemanticsObject::checkSemantics(const Semantics::FilterFunction& filter, bool onlyEmitted) const {
    auto& semantics = onlyEmitted ? m_emittedSemantics : m_semantics;
    for (auto& sem : semantics) {
        if (filter(sem))
            return true;
    }
    return false;
}

std::list<Semantics*> SemanticsObject::findSemantics(const Semantics::FilterFunction& filter) const {
    std::list<Semantics*> result;
    for (auto& semantics : m_semantics) {
        if (filter(semantics))
            result.push_back(semantics);
    }
    return result;
}

VariableSemObj::VariableSemObj(const ircode::Variable* variable, const std::shared_ptr<SemanticsContext>& context)
    : m_variable(variable), m_context(context)
{}

SemanticsObject::Id VariableSemObj::getId() const {
    return GetId(m_variable);
}

void VariableSemObj::bindTo(SemanticsObject* obj) {
    m_relatedObjects.insert(obj);
}

void VariableSemObj::unbindFrom(SemanticsObject* obj) {
    m_relatedObjects.erase(obj);
}

SemanticsContextOperations VariableSemObj::getRelatedOperations() const {
    SemanticsContextOperations result;
    for (auto op : m_variable->getOperations()) {
        result.insert({m_context, op});
    }
    return result;
}

const ircode::Variable* VariableSemObj::getVariable() const {
    return m_variable;
}

SemanticsObject::Id VariableSemObj::GetId(const ircode::Variable* var) {
    return reinterpret_cast<size_t>(var);
}

SymbolSemObj::SymbolSemObj(const Symbol* symbol)
    : m_symbol(symbol)
{}

SemanticsObject::Id SymbolSemObj::getId() const {
    return GetId(m_symbol);
}

void SymbolSemObj::bindTo(SemanticsObject* obj) {
    if (auto varObj = dynamic_cast<VariableSemObj*>(obj)) {
        m_relatedVarObjects.insert(varObj);
    }
}

void SymbolSemObj::unbindFrom(SemanticsObject* obj) {
    if (auto varObj = dynamic_cast<VariableSemObj*>(obj)) {
        m_relatedVarObjects.erase(varObj);
    }
}

SemanticsContextOperations SymbolSemObj::getRelatedOperations() const {
    SemanticsContextOperations result;
    for (auto varObj : m_relatedVarObjects) {
        auto relOps = varObj->getRelatedOperations();
        result.insert(relOps.begin(), relOps.end());
    }
    return result;
}

SemanticsObject::Id SymbolSemObj::GetId(const Symbol* symbol) {
    return reinterpret_cast<size_t>(symbol);
}

SymbolTableSemObj::SymbolTableSemObj(const SymbolTable* symbolTable)
    : m_symbolTable(symbolTable)
{}

SemanticsObject::Id SymbolTableSemObj::getId() const {
    return GetId(m_symbolTable);
}

void SymbolTableSemObj::bindTo(SemanticsObject* obj) {
    if (auto varObj = dynamic_cast<VariableSemObj*>(obj)) {
        Offset offset = varObj->getVariable()->getLinearExpr().getConstTermValue();
        auto it = m_offsetToRelatedVarObjects.find(offset);
        if (it == m_offsetToRelatedVarObjects.end()) {
            it = m_offsetToRelatedVarObjects.insert(
                std::make_pair(offset, std::set<VariableSemObj*>())).first;
        }
        it->second.insert(varObj);
    }
}

void SymbolTableSemObj::unbindFrom(SemanticsObject* obj) {
    if (auto varObj = dynamic_cast<VariableSemObj*>(obj)) {
        Offset offset = varObj->getVariable()->getLinearExpr().getConstTermValue();
        auto it = m_offsetToRelatedVarObjects.find(offset);
        if (it != m_offsetToRelatedVarObjects.end()) {
            it->second.erase(varObj);
        }
    }
}

SemanticsContextOperations SymbolTableSemObj::getRelatedOperations(Offset offset) const {
    SemanticsContextOperations result;
    auto it = m_offsetToRelatedVarObjects.find(offset);
    if (it != m_offsetToRelatedVarObjects.end()) {
        for (auto varObj : it->second) {
            auto relOps = varObj->getRelatedOperations();
            result.insert(relOps.begin(), relOps.end());
        }
    }
    return result;
}

SemanticsObject::Id SymbolTableSemObj::GetId(const SymbolTable* symbolTable) {
    return reinterpret_cast<size_t>(symbolTable);
}

FuncReturnSemObj::FuncReturnSemObj(const SignatureDataType* signatureDt)
    : m_signatureDt(signatureDt)
{}

SemanticsObject::Id FuncReturnSemObj::getId() const {
    return GetId(m_signatureDt);
}

void FuncReturnSemObj::bindTo(SemanticsObject* obj) {
    if (auto varObj = dynamic_cast<VariableSemObj*>(obj)) {
        m_relatedVarObjects.insert(varObj);
    }
}

void FuncReturnSemObj::unbindFrom(SemanticsObject* obj) {
    if (auto varObj = dynamic_cast<VariableSemObj*>(obj)) {
        m_relatedVarObjects.erase(varObj);
    }
}

SemanticsContextOperations FuncReturnSemObj::getRelatedOperations() const {
    SemanticsContextOperations result;
    for (auto varObj : m_relatedVarObjects) {
        auto relOps = varObj->getRelatedOperations();
        result.insert(relOps.begin(), relOps.end());
    }
    return result;
}