#include "Decompiler/Semantics/SemanticsPropagator.h"
#include "Decompiler/Semantics/SemanticsManager.h"
#include "Core/DataType/ScalarDataType.h"
#include "Core/DataType/PointerDataType.h"
#include "Core/DataType/StructureDataType.h"

using namespace sda;
using namespace sda::decompiler;

SemanticsPropagator::SemanticsPropagator(SemanticsManager* semManager)
    : m_semManager(semManager)
{}

SemanticsManager* SemanticsPropagator::getManager() const {
    return m_semManager;
}

VariableSemObj* SemanticsPropagator::getOrCreateVarObject(std::shared_ptr<ircode::Variable> var) const {
    auto id = VariableSemObj::GetId(var.get());
    if (auto obj = getManager()->getObject<VariableSemObj>(id))
        return obj;
    auto newObj = getManager()->addObject(std::make_unique<VariableSemObj>(var.get()));
    return dynamic_cast<VariableSemObj*>(newObj);
}

void SemanticsPropagator::bindEachOther(SemanticsObject* obj1, SemanticsObject* obj2) const {
    obj1->bindTo(obj2);
    obj2->bindTo(obj1);
}

void SemanticsPropagator::propagateTo(
    SemanticsObject* fromObj,
    SemanticsObject* toObj,
    Semantics::FilterFunction filter,
    std::set<SemanticsObject*>& nextObjs) const
{
    if (getManager()->isSimiliarityConsidered()) {
        auto toObjSemantics = toObj->findSemantics(filter);
        auto simFilter = [toObjSemantics](const Semantics* sem) {
            for (auto sem2 : toObjSemantics) {
                if (sem == sem2 || sem->isSimiliarTo(sem2))
                    return false;
            }
            return true;
        };
        filter = Semantics::FilterAnd(filter, simFilter);
    }
    if (fromObj->propagateTo(toObj, filter)) // todo: remove
        nextObjs.insert(toObj);
}

void BaseSemanticsPropagator::propagate(
    const SemanticsContext* ctx,
    const ircode::Operation* op,
    std::set<SemanticsObject*>& nextObjs)
{
    auto output = op->getOutput();

    if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(op)) {
        auto input = unaryOp->getInput();
        if (unaryOp->getId() == ircode::OperationId::LOAD) {
            if (auto regValue = std::dynamic_pointer_cast<ircode::Register>(input)) {
                const auto& reg = regValue->getRegister();
                auto signatureDt = ctx->functionSymbol->getSignature();
                auto it = signatureDt->getStorages().find({
                    CallingConvention::Storage::Read,
                    reg.getRegId()
                });
                if (it != signatureDt->getStorages().end()) {
                    // if it is a parameter
                    const auto& storageInfo = it->second;
                    auto paramIdx = storageInfo.paramIdx;
                    auto paramSymbol = signatureDt->getParameters()[paramIdx];
                    if (auto paramSymbolObj = getSymbolObject(paramSymbol)) {
                        auto outputVarObj = getOrCreateVarObject(output);
                        bindEachOther(paramSymbolObj, outputVarObj);
                        propagateTo(paramSymbolObj, outputVarObj, DataTypeSemantics::Filter(), nextObjs);
                        propagateTo(outputVarObj, paramSymbolObj, DataTypeSemantics::Filter(), nextObjs);
                    }
                } else {
                    // if it is a stack or global variable
                    auto symbolTable = ctx->functionSymbol->getStackSymbolTable();
                    if (reg.getRegId() == Register::InstructionPointerId)
                        symbolTable = ctx->globalSymbolTable;
                    if (symbolTable) {
                        if (auto symbolTableObj = getSymbolTableObject(symbolTable)) {
                            auto outputVarObj = getOrCreateVarObject(output);
                            bindEachOther(symbolTableObj, outputVarObj);
                            propagateTo(symbolTableObj, outputVarObj, SymbolTableSemantics::Filter(), nextObjs);
                        }
                    }
                }
            }
        }
        else if (unaryOp->getId() == ircode::OperationId::COPY) {
            if (auto variable = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                auto inputVarObj = getOrCreateVarObject(variable);
                auto outputVarObj = getOrCreateVarObject(output);
                propagateTo(inputVarObj, outputVarObj, Semantics::FilterAll(), nextObjs);
                propagateTo(outputVarObj, inputVarObj, Semantics::FilterAll(), nextObjs);
            }
        }
        else if (unaryOp->getId() == ircode::OperationId::INT_2COMP) {
            auto signedScalarDt = getScalarDataType(ScalarType::SignedInt, output->getSize());
            setDataTypeFor(input, signedScalarDt, nextObjs);
            setDataTypeFor(output, signedScalarDt, nextObjs);
        }
        else if (unaryOp->getId() == ircode::OperationId::BOOL_NEGATE) {
            auto booleanDt = findDataType("bool");
            setDataTypeFor(input, booleanDt, nextObjs);
            setDataTypeFor(output, booleanDt, nextObjs);
        }
        else if (unaryOp->getId() == ircode::OperationId::FLOAT_NEG ||
                 unaryOp->getId() == ircode::OperationId::FLOAT_ABS ||
                 unaryOp->getId() == ircode::OperationId::FLOAT_SQRT) {
            auto floatScalarDt = getScalarDataType(ScalarType::FloatingPoint, output->getSize());
            setDataTypeFor(input, floatScalarDt, nextObjs);
            setDataTypeFor(output, floatScalarDt, nextObjs);
        }
    }
    else if (auto binaryOp = dynamic_cast<const ircode::BinaryOperation*>(op)) {
        auto input1 = binaryOp->getInput1();
        auto input2 = binaryOp->getInput2();

        if (binaryOp->getId() == ircode::OperationId::INT_ADD ||
            binaryOp->getId() == ircode::OperationId::INT_SUB ||
            binaryOp->getId() == ircode::OperationId::INT_MULT ||
            binaryOp->getId() == ircode::OperationId::INT_DIV ||
            binaryOp->getId() == ircode::OperationId::INT_REM)
        {
            auto outputVarObj = getOrCreateVarObject(output);
            if (auto inputVar1 = std::dynamic_pointer_cast<ircode::Variable>(input1)) {
                if (auto inputVar2 = std::dynamic_pointer_cast<ircode::Variable>(input2)) {
                    auto inputVarObj1 = getOrCreateVarObject(inputVar1);
                    auto inputVarObj2 = getOrCreateVarObject(inputVar2);
                    auto signedScalarDt = getScalarDataType(ScalarType::SignedInt, output->getSize());
                    auto filter = DataTypeSemantics::Filter(signedScalarDt);
                    propagateTo(inputVarObj1, outputVarObj, filter, nextObjs);
                    propagateTo(inputVarObj2, outputVarObj, filter, nextObjs);
                }
            }

            if (binaryOp->getId() == ircode::OperationId::INT_ADD ||
                binaryOp->getId() == ircode::OperationId::INT_MULT)
            {
                auto linearExpr = output->getLinearExpr();
                Offset offset = linearExpr.getConstTermValue();
                for (auto& term : linearExpr.getTerms()) {
                    if (!term.canBePointer())
                        continue;
                    if (auto termVar = std::dynamic_pointer_cast<ircode::Variable>(term.value)) {
                        auto termVarObj = getOrCreateVarObject(termVar);
                        auto filter = Semantics::FilterOr(DataTypeSemantics::Filter(), SymbolTableSemantics::Filter());
                        auto semantics = termVarObj->findSemantics(filter);
                        for (auto sem : semantics) {
                            SymbolTable* symbolTable = nullptr;
                            if (auto symbolTableSem = dynamic_cast<SymbolTableSemantics*>(sem)) {
                                symbolTable = symbolTableSem->getSymbolTable();
                            } else if (auto dataTypeSem = dynamic_cast<DataTypeSemantics*>(sem)) {
                                if (auto pointerDt = dynamic_cast<const PointerDataType*>(dataTypeSem->getDataType()))
                                    if (auto structDt = dynamic_cast<const StructureDataType*>(pointerDt->getPointedType()))
                                        symbolTable = structDt->getSymbolTable();
                            }
                            
                            if (symbolTable) {
                                Symbol* symbol = nullptr;
                                if (symbolTable == ctx->globalSymbolTable ||
                                    symbolTable == ctx->functionSymbol->getStackSymbolTable())
                                {
                                    CallingConvention::Storage storage;
                                    storage.useType = CallingConvention::Storage::Read;
                                    storage.registerId = Register::StackPointerId;
                                    if (symbolTable == ctx->globalSymbolTable)
                                        storage.registerId = Register::InstructionPointerId;
                                    storage.offset = offset;
                                    auto signatureDt = ctx->functionSymbol->getSignature();
                                    auto it = signatureDt->getStorages().find(storage);
                                    if (it != signatureDt->getStorages().end()) {
                                        const auto& storageInfo = it->second;
                                        auto paramIdx = storageInfo.paramIdx;
                                        symbol = signatureDt->getParameters()[paramIdx];
                                    }
                                }
                                if (!symbol) {
                                    symbol = symbolTable->getSymbolAt(offset);
                                }

                                if (symbol) {
                                    if (auto symbolObj = getSymbolObject(symbol)) {
                                        bindEachOther(symbolObj, outputVarObj);
                                        auto newSem = createDataTypeSemantics(outputVarObj, symbol->getDataType()->getPointerTo());
                                        sem->addSuccessors(newSem);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (binaryOp->getId() == ircode::OperationId::INT_SDIV ||
                binaryOp->getId() == ircode::OperationId::INT_SREM)
        {
            auto signedScalarDt = getScalarDataType(ScalarType::SignedInt, output->getSize());
            setDataTypeFor(output, signedScalarDt, nextObjs);
        }
        else if (binaryOp->getId() >= ircode::OperationId::INT_EQUAL &&
                binaryOp->getId() <= ircode::OperationId::INT_LESSEQUAL ||
                binaryOp->getId() >= ircode::OperationId::FLOAT_EQUAL &&
                binaryOp->getId() <= ircode::OperationId::FLOAT_LESSEQUAL)
        {
            auto booleanDt = findDataType("bool");
            setDataTypeFor(output, booleanDt, nextObjs);
            if (binaryOp->getId() >= ircode::OperationId::FLOAT_EQUAL &&
                binaryOp->getId() <= ircode::OperationId::FLOAT_LESSEQUAL)
                {
                    auto floatScalarDt = getScalarDataType(ScalarType::FloatingPoint, output->getSize());
                    setDataTypeFor(input1, floatScalarDt, nextObjs);
                    setDataTypeFor(input2, floatScalarDt, nextObjs);
                }
        }
        else if (binaryOp->getId() >= ircode::OperationId::BOOL_NEGATE &&
                binaryOp->getId() <= ircode::OperationId::BOOL_OR)
        {
            auto booleanDt = findDataType("bool");
            setDataTypeFor(input1, booleanDt, nextObjs);
            setDataTypeFor(input2, booleanDt, nextObjs);
            setDataTypeFor(output, booleanDt, nextObjs);
        }
        else if (binaryOp->getId() >= ircode::OperationId::FLOAT_ADD &&
                binaryOp->getId() <= ircode::OperationId::FLOAT_SQRT)
        {
            auto floatScalarDt = getScalarDataType(ScalarType::FloatingPoint, output->getSize());
            setDataTypeFor(input1, floatScalarDt, nextObjs);
            setDataTypeFor(input2, floatScalarDt, nextObjs);
            setDataTypeFor(output, floatScalarDt, nextObjs);
        }
    }
}

SymbolSemObj* BaseSemanticsPropagator::getSymbolObject(const Symbol* symbol) const {
    auto id = SymbolSemObj::GetId(symbol);
    if (auto obj = getManager()->getObject<SymbolSemObj>(id))
        return obj;
    return nullptr;
}

SymbolTableSemObj* BaseSemanticsPropagator::getSymbolTableObject(const SymbolTable* symbolTable) const {
    auto id = SymbolTableSemObj::GetId(symbolTable);
    if (auto obj = getManager()->getObject<SymbolTableSemObj>(id))
        return obj;
    return nullptr;
}

FuncReturnSemObj* BaseSemanticsPropagator::getFuncReturnObject(const SignatureDataType* signatureDt) const {
    auto id = FuncReturnSemObj::GetId(signatureDt);
    if (auto obj = getManager()->getObject<FuncReturnSemObj>(id))
        return obj;
    return nullptr;
}

DataType* BaseSemanticsPropagator::findDataType(const std::string& name) const {
    auto dataType = getManager()->getContext()->getDataTypes()->getByName(name);
    assert(dataType);
    return dataType;
}

ScalarDataType* BaseSemanticsPropagator::getScalarDataType(ScalarType scalarType, size_t size) const {
    return getManager()->getContext()->getDataTypes()->getScalar(scalarType, size);
}

DataTypeSemantics* BaseSemanticsPropagator::createDataTypeSemantics(SemanticsObject* sourceObject, const DataType* dataType, size_t uncertaintyDegree) const {
    auto sem = getManager()->addSemantics(std::make_unique<DataTypeSemantics>(sourceObject, dataType, uncertaintyDegree));
    return dynamic_cast<DataTypeSemantics*>(sem);
}

void BaseSemanticsPropagator::setDataTypeFor(std::shared_ptr<ircode::Value> value, const DataType* dataType, std::set<SemanticsObject*>& nextObjs) const {
    if (auto var = std::dynamic_pointer_cast<ircode::Variable>(value)) {
        auto varObj = getOrCreateVarObject(var);
        bool onlyEmitted = !getManager()->isSimiliarityConsidered();
        if (!varObj->checkSemantics(DataTypeSemantics::Filter(dataType), onlyEmitted)) {
            auto sem = createDataTypeSemantics(varObj, dataType);
            nextObjs.insert(varObj);
        }
    }
}

std::set<SymbolTable*> BaseSemanticsPropagator::getAllSymbolTables(const ircode::LinearExpression& linearExpr) const {
    std::set<SymbolTable*> symbolTables;
    for (auto& term : linearExpr.getTerms()) {
        if (!term.canBePointer())
            continue;
        if (auto termVar = std::dynamic_pointer_cast<ircode::Variable>(term.value)) {
            auto termVarObj = getOrCreateVarObject(termVar);
            auto filter = Semantics::FilterOr(DataTypeSemantics::Filter(), SymbolTableSemantics::Filter());
            auto semantics = termVarObj->findSemantics(filter);
            for (auto sem : semantics) {
                if (auto symbolTableSem = dynamic_cast<SymbolTableSemantics*>(sem)) {
                    symbolTables.insert(symbolTableSem->getSymbolTable());
                } else if (auto dataTypeSem = dynamic_cast<DataTypeSemantics*>(sem)) {
                    if (auto pointerDt = dynamic_cast<const PointerDataType*>(dataTypeSem->getDataType()))
                        if (auto structDt = dynamic_cast<const StructureDataType*>(pointerDt->getPointedType()))
                            symbolTables.insert(structDt->getSymbolTable());
                }
            }
        }
    }
    return symbolTables;
}