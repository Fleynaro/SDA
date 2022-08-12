#include "Decompiler/Semantics/SemanticsObject.h"

using namespace sda;
using namespace sda::decompiler;

void SemanticsObject::addSemantics(Semantics* semantics) {
    m_semantics.insert(semantics);
}

bool SemanticsObject::checkSemantics(const Semantics::FilterFunction& filter) const {
    for (auto& semantics : m_semantics) {
        if (filter(semantics))
            return true;
    }
    return false;
}

bool SemanticsObject::propagateTo(SemanticsObject* obj, const Semantics::FilterFunction& filter) {
    bool result = false;
    for (auto semantics : m_semantics) {
        if (filter(semantics)) {
            auto pair = obj->m_semantics.insert(semantics);
            result |= pair.second;
        }
    }
    return result;
}

VariableSemObj::VariableSemObj(const ircode::Variable* variable)
    : m_variable(variable)
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