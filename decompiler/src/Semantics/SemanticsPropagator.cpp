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

VariableSemObj* SemanticsPropagator::getOrCreateVarObject(std::shared_ptr<ircode::Variable> var) {
    auto id = VariableSemObj::GetId(var.get());
    if (auto obj = getManager()->getObject<VariableSemObj>(id))
        return obj;
    auto newObj = getManager()->addObject(std::make_unique<VariableSemObj>(var.get()));
    return dynamic_cast<VariableSemObj*>(newObj);
}

void SemanticsPropagator::bind(SemanticsObject* obj1, SemanticsObject* obj2) {
    obj1->bindTo(obj2);
    obj2->bindTo(obj1);
}

void BaseSemanticsPropagator::propagate(
    const SemanticsContext* ctx,
    const ircode::Operation* op,
    std::list<SemanticsObject*>& nextObjs)
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
                        if (auto outputVarObj = getOrCreateVarObject(output)) {
                            bind(paramSymbolObj, outputVarObj);
                            if (paramSymbolObj->propagateTo(outputVarObj, DataTypeSemantics::Filter()))
                                nextObjs.push_back(outputVarObj);
                        }
                    }
                } else {
                    // if it is a stack or global variable
                    auto symbolTable = ctx->functionSymbol->getStackSymbolTable();
                    if (reg.getRegId() == Register::InstructionPointerId)
                        symbolTable = ctx->globalSymbolTable;
                    if (symbolTable) {
                        if (auto symbolTableObj = getSymbolTableObject(symbolTable)) {
                            if (auto outputVarObj = getOrCreateVarObject(output)) {
                                bind(symbolTableObj, outputVarObj);
                                if (symbolTableObj->propagateTo(outputVarObj, DataTypeSemantics::Filter()))
                                    nextObjs.push_back(outputVarObj);
                            }
                        }
                    }
                }
            }
        }
        else if (unaryOp->getId() == ircode::OperationId::COPY) {
            if (auto variable = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                if (auto inputVarObj = getOrCreateVarObject(variable)) {
                    if (auto outputVarObj = getOrCreateVarObject(output)) {
                        if (outputVarObj->propagateTo(inputVarObj))
                            nextObjs.push_back(inputVarObj);
                    }
                }
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
            if (input1->getDataType()->isScalar(ScalarType::SignedInt) ||
                input2->getDataType()->isScalar(ScalarType::SignedInt))
            {
                auto signedScalarDt = getScalarDataType(ScalarType::SignedInt, output->getSize());
                setDataTypeFor(output, signedScalarDt, nextObjs);
            }

            if (binaryOp->getId() == ircode::OperationId::INT_ADD ||
                binaryOp->getId() == ircode::OperationId::INT_MULT)
            {
                auto linearExpr = output->getLinearExpr();
                auto baseValue = linearExpr.getBaseValue();
                if (baseValue->getDataType()->isPointer()) {
                    if (auto symbolTable = baseValue->getSymbolTable()) {
                        Offset offset = linearExpr.getConstTermValue();
                        
                        CallingConvention::Storage storage;
                        storage.useType = CallingConvention::Storage::Read;
                        storage.registerId = Register::StackPointerId;
                        if (symbolTable == m_globalSymbolTable)
                            storage.registerId = Register::InstructionPointerId;
                        storage.offset = offset;
                        auto it = m_storages.find(storage);
                        if (it != m_storages.end()) {
                            const auto& storageInfo = it->second;
                            auto paramIdx = storageInfo.paramIdx;
                            auto paramSymbol = m_signatureDt->getParameters()[paramIdx];
                            output->setDataType(paramSymbol->getDataType()->getPointerTo());
                        }

                        if (auto symbol = symbolTable->getSymbolAt(offset)) {
                            output->setDataType(symbol->getDataType()->getPointerTo());
                        } else {
                            auto voidDt = findDataType("void");
                            output->setDataType(voidDt->getPointerTo());
                        }
                    } else {
                        output->setDataType(baseValue->getDataType());
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

DataTypeSemantics* BaseSemanticsPropagator::createDataTypeSemantics(const DataType* dataType, const SymbolTable* symbolTable) const {
    if (!symbolTable) {
        if (auto pointerDt = dynamic_cast<const PointerDataType*>(dataType))
            if (auto structDt = dynamic_cast<const StructureDataType*>(pointerDt->getPointedType()))
                symbolTable = structDt->getSymbolTable();
    }
    auto sem = getManager()->addSemantics(std::make_unique<DataTypeSemantics>(dataType, symbolTable));
    return dynamic_cast<DataTypeSemantics*>(sem);
}

void BaseSemanticsPropagator::setDataTypeFor(std::shared_ptr<ircode::Value> value, const DataType* dataType, std::list<SemanticsObject*>& nextObjs) {
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
    if (auto var = std::dynamic_pointer_cast<ircode::Variable>(value)) {
        if (auto varObj = getOrCreateVarObject(var)) {
            if (!varObj->checkSemantics(DataTypeSemantics::Filter(filter))) { // todo: need check existing? hard to calculate stats!
                auto sem = createDataTypeSemantics(dataType);
                varObj->addSemantics(sem);
                nextObjs.push_back(varObj);
            }
        }
    }
}