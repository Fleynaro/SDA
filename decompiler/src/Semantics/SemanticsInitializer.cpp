#include "SDA/Decompiler/Semantics/SemanticsInitializer.h"
#include "SDA/Core/DataType/StructureDataType.h"
#include "SDA/Core/Symbol/FunctionSymbol.h"

using namespace sda;
using namespace sda::decompiler;

BaseSemanticsInitializer::BaseSemanticsInitializer(SemanticsManager* semManager)
    : m_semManager(semManager)
{}

void BaseSemanticsInitializer::addSymbolTable(SymbolTable* symbolTable) {
    if (m_semManager->getObject(SymbolTableSemObj::GetId(symbolTable)))
        return;

    auto obj = new SymbolTableSemObj(m_semManager, symbolTable);
    new SymbolTableSemantics(obj, getSourceInfo(), symbolTable);

    auto allSymbols = symbolTable->getAllSymbols();
    for (auto symbolInfo : allSymbols) {
        addSymbol(symbolInfo.symbol);
    }
}

void BaseSemanticsInitializer::addSymbol(Symbol* symbol) {
    if (m_semManager->getObject(SymbolSemObj::GetId(symbol)))
        return;

    auto obj = new SymbolSemObj(m_semManager, symbol);
    auto symbolDt = symbol->getDataType();
    new DataTypeSemantics(obj, getSourceInfo(), symbolDt);
    addDataType(symbolDt);

    if (auto funcSymbol = dynamic_cast<FunctionSymbol*>(symbol)) {
        addSymbolTable(funcSymbol->getStackSymbolTable());
        addSymbolTable(funcSymbol->getInstructionSymbolTable());
    }
}

void BaseSemanticsInitializer::addFuncReturn(SignatureDataType* signatureDt) {
    auto returnDt = signatureDt->getReturnType();
    if (m_semManager->getObject(FuncReturnSemObj::GetId(signatureDt)) || returnDt->isVoid())
        return;

    auto obj = new FuncReturnSemObj(m_semManager, signatureDt);
    new DataTypeSemantics(obj, getSourceInfo(), returnDt);
    addDataType(returnDt);
}

void BaseSemanticsInitializer::addDataType(DataType* dataType) {
    auto baseDt = dataType->getBaseType();
    if (auto structDt = dynamic_cast<StructureDataType*>(baseDt)) {
       auto allSymbols = structDt->getSymbolTable()->getAllSymbols();
        for (auto symbolInfo : allSymbols) {
            addSymbol(symbolInfo.symbol);
        }
    } else if (auto signatureDt = dynamic_cast<SignatureDataType*>(baseDt)) {
        for (auto paramSymbol : signatureDt->getParameters()) {
            addSymbol(paramSymbol);
        }
    }
}

std::shared_ptr<Semantics::SourceInfo> BaseSemanticsInitializer::getSourceInfo() const {
    auto sourceInfo = std::make_shared<Semantics::SourceInfo>();
    sourceInfo->creatorType = Semantics::User;
    return sourceInfo;
}