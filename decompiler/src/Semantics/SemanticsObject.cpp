#include "SDA/Decompiler/Semantics/SemanticsObject.h"
#include "SDA/Decompiler/Semantics/SemanticsManager.h"
#include "rang.hpp"

using namespace sda;
using namespace sda::decompiler;

void SemanticsObject::disconnect() {
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
        if (filter(sem.get()))
            return true;
    }
    return false;
}

std::list<Semantics*> SemanticsObject::findSemantics(const Semantics::FilterFunction& filter) const {
    std::list<Semantics*> result;
    for (auto& sem : m_semantics) {
        if (filter(sem.get()))
            result.push_back(sem.get());
    }
    return result;
}

void SemanticsObject::removeSemantics(Semantics* semantics, SemanticsContextOperations& operations) {
    std::list<Semantics*> semanticsToRemove = {semantics};
    std::list<SemanticsObject*> sourceHolders;
    while (!semanticsToRemove.empty()) {
        auto semToRemove = semanticsToRemove.front();
        semanticsToRemove.pop_front();
        
        if (semToRemove->getPredecessors().size() >= 2 || semToRemove == semantics)
            sourceHolders.push_back(semToRemove->getHolder());

        for (auto& nextSem : semToRemove->getSuccessors())
            semanticsToRemove.push_back(nextSem);

        semToRemove->disconnect();
        m_semantics.remove_if([semToRemove](const std::unique_ptr<Semantics>& sem) {
            return sem.get() == semToRemove;
        });
    }

    for (auto holder : sourceHolders)
        holder->getAllRelatedOperations(operations);
}

void SemanticsObject::print(std::ostream& out) const {
    out << ": " << rang::fg::reset;
    for (auto& sem : m_semantics) {
        sem->print(out);
        if (sem != m_semantics.back())
            out << ", ";
    }
}

void SemanticsObject::addToManager(SemanticsManager* semManager) {
    semManager->m_objects[getId()] = std::unique_ptr<SemanticsObject>(this);
}

VariableSemObj::VariableSemObj(
    SemanticsManager* semManager,
    const ircode::Variable* variable,
    const std::shared_ptr<SemanticsContext>& context)
    : m_variable(variable)
    , m_context(context)
{
    addToManager(semManager);
}

SemanticsObject::Id VariableSemObj::getId() const {
    return GetId(m_variable);
}

std::string VariableSemObj::getName() const {
    return m_variable->getName();
}

void VariableSemObj::getAllRelatedOperations(SemanticsContextOperations& operations) const {
    for (auto op : m_variable->getOperations()) {
        operations.insert({m_context, op});
    }
}

void VariableSemObj::print(std::ostream& out) const {
    out << rang::fgB::gray << getName();
    SemanticsObject::print(out);
}

const ircode::Variable* VariableSemObj::getVariable() const {
    return m_variable;
}

SemanticsObject::Id VariableSemObj::GetId(const ircode::Variable* var) {
    return reinterpret_cast<size_t>(var);
}

SymbolSemObj::SymbolSemObj(SemanticsManager* semManager, const Symbol* symbol)
    : m_symbol(symbol)
{
    addToManager(semManager);
}

SemanticsObject::Id SymbolSemObj::getId() const {
    return GetId(m_symbol);
}

std::string SymbolSemObj::getName() const {
    return m_symbol->getName();
}

void SymbolSemObj::print(std::ostream& out) const {
    if (dynamic_cast<const FunctionParameterSymbol*>(m_symbol))
        out << rang::fgB::red;
    else
        out << rang::fgB::blue;
    out << getName();
    SemanticsObject::print(out);
}

SemanticsObject::Id SymbolSemObj::GetId(const Symbol* symbol) {
    return reinterpret_cast<size_t>(symbol);
}

SymbolTableSemObj::SymbolTableSemObj(SemanticsManager* semManager, const SymbolTable* symbolTable)
    : m_symbolTable(symbolTable)
{
    addToManager(semManager);
}

SemanticsObject::Id SymbolTableSemObj::getId() const {
    return GetId(m_symbolTable);
}

std::string SymbolTableSemObj::getName() const {
    return m_symbolTable->getName();
}

void SymbolTableSemObj::print(std::ostream& out) const {
    out << getName();
    SemanticsObject::print(out);
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

FuncReturnSemObj::FuncReturnSemObj(SemanticsManager* semManager, const SignatureDataType* signatureDt)
    : m_signatureDt(signatureDt)
{
    addToManager(semManager);
}

SemanticsObject::Id FuncReturnSemObj::getId() const {
    return GetId(m_signatureDt);
}

std::string FuncReturnSemObj::getName() const {
    return m_signatureDt->getName() + "<return>";
}

void FuncReturnSemObj::print(std::ostream& out) const {
    out << getName();
    SemanticsObject::print(out);
}

SemanticsObject::Id FuncReturnSemObj::GetId(const SignatureDataType* signatureDt) {
    return reinterpret_cast<size_t>(signatureDt);
}