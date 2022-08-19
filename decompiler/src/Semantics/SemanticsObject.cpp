#include "Decompiler/Semantics/SemanticsObject.h"

using namespace sda;
using namespace sda::decompiler;

SemanticsObject::~SemanticsObject() {
    assert(m_semantics.empty());
    for (auto obj : m_allRelatedObjects)
        obj->m_allRelatedObjects.erase(this);
}

void SemanticsObject::bindTo(SemanticsObject* obj) {
    m_allRelatedObjects.insert(obj);
}

void SemanticsObject::unbindFrom(SemanticsObject* obj) {
    m_allRelatedObjects.erase(obj);
}

void SemanticsObject::getAllRelatedOperations(SemanticsContextOperations& operations) const {
    for (auto obj : m_allRelatedObjects)
        obj->getAllRelatedOperations(operations);
}

bool SemanticsObject::checkSemantics(const Semantics::FilterFunction& filter) const {
    for (auto& sem : m_semantics) {
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

void VariableSemObj::getAllRelatedOperations(SemanticsContextOperations& operations) const {
    for (auto op : m_variable->getOperations()) {
        operations.insert({m_context, op});
    }
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
    SemanticsObject::bindTo(obj);
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
    SemanticsObject::unbindFrom(obj);
    if (auto varObj = dynamic_cast<VariableSemObj*>(obj)) {
        Offset offset = varObj->getVariable()->getLinearExpr().getConstTermValue();
        auto it = m_offsetToRelatedVarObjects.find(offset);
        if (it != m_offsetToRelatedVarObjects.end()) {
            it->second.erase(varObj);
        }
    }
}

void SymbolTableSemObj::getRelatedOperationsAtOffset(SemanticsContextOperations& operations, Offset offset) const {
    auto it = m_offsetToRelatedVarObjects.find(offset);
    if (it != m_offsetToRelatedVarObjects.end()) {
        for (auto varObj : it->second) {
            varObj->getAllRelatedOperations(operations);
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