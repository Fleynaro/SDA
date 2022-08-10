#include "Decompiler/Semantics/IRcodeDataTypePropagator.h"
#include "Core/DataType/PointerDataType.h"
#include "Core/DataType/ScalarDataType.h"
#include "Core/DataType/StructureDataType.h"
#include "Core/Symbol/Symbol.h"

using namespace sda;
using namespace sda::decompiler;

IRcodeDataTypePropagator::IRcodeDataTypePropagator(
    Context* context,
    SignatureDataType* signatureDt,
    SymbolTable* globalSymbolTable,
    SymbolTable* stackSymbolTable,
    SymbolTable* instructionSymbolTable)
    : m_context(context)
    , m_signatureDt(signatureDt)
    , m_globalSymbolTable(globalSymbolTable)
    , m_stackSymbolTable(stackSymbolTable)
    , m_instructionSymbolTable(instructionSymbolTable)
{
    m_storages = signatureDt->getStorages();
}

void IRcodeDataTypePropagator::propagate(const ircode::Operation* operation) {
    auto output = operation->getOutput();
    setDefaultDataType(output);

    if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(operation)) {
        auto input = unaryOp->getInput();
        setDefaultDataType(input);
        if (unaryOp->getId() == ircode::OperationId::LOAD) {
            if (auto regValue = std::dynamic_pointer_cast<ircode::Register>(input)) {
                const auto& reg = regValue->getRegister();
                auto it = m_storages.find({
                    CallingConvention::Storage::Read,
                    reg.getRegId()
                });
                if (it != m_storages.end()) {
                    // if it is a parameter
                    const auto& storageInfo = it->second;
                    auto paramIdx = storageInfo.paramIdx;
                    auto paramSymbol = m_signatureDt->getParameters()[paramIdx];
                    auto paramDt = paramSymbol->getDataType();
                    input->setDataType(paramDt->getPointerTo());
                    output->setDataType(paramDt);
                    if (auto structDt = dynamic_cast<StructureDataType*>(paramDt))
                        output->setSymbolTable(structDt->getSymbolTable());
                } else {
                    // if it is a stack or global variable
                    SymbolTable* symbolTable = m_stackSymbolTable;
                    if (reg.getRegId() == Register::InstructionPointerId)
                        symbolTable = m_globalSymbolTable;
                    if (symbolTable) {
                        auto voidDt = findDataType("void");
                        input->setDataType(voidDt->getPointerTo()->getPointerTo()); // void**
                        output->setDataType(voidDt->getPointerTo()); // void*
                        output->setSymbolTable(symbolTable);
                    }
                }
            }
            else if (auto variable = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                if (auto pointerDt = dynamic_cast<PointerDataType*>(variable->getDataType())) {
                    output->setDataType(pointerDt->getPointedType());
                    if (auto structDt = dynamic_cast<StructureDataType*>(pointerDt->getPointedType()))
                        output->setSymbolTable(structDt->getSymbolTable());
                }
            }
        }
        else if (unaryOp->getId() == ircode::OperationId::COPY) {
            if (auto variable = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                output->setDataType(input->getDataType());
                output->setSymbolTable(input->getSymbolTable());
            }
        }
        else if (unaryOp->getId() == ircode::OperationId::INT_2COMP) {
            auto signedScalarDt = getScalarDataType(ScalarType::SignedInt, output->getSize());
            assert(signedScalarDt);
            output->setDataType(signedScalarDt);
        }
        else if (unaryOp->getId() == ircode::OperationId::BOOL_NEGATE) {
            auto booleanDt = findDataType("bool");
            output->setDataType(booleanDt);
        }
        else if (unaryOp->getId() == ircode::OperationId::FLOAT_NEG) {
            auto floatScalarDt = getScalarDataType(ScalarType::FloatingPoint, output->getSize());
            assert(floatScalarDt);
            output->setDataType(floatScalarDt);
        }
    }
    else if (auto binaryOp = dynamic_cast<const ircode::BinaryOperation*>(operation)) {
        auto input1 = binaryOp->getInput1();
        auto input2 = binaryOp->getInput2();
        setDefaultDataType(input1);
        setDefaultDataType(input2);

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
                assert(signedScalarDt);
                output->setDataType(signedScalarDt);
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
            assert(signedScalarDt);
            output->setDataType(signedScalarDt);
        }
        else if (binaryOp->getId() >= ircode::OperationId::INT_EQUAL &&
                binaryOp->getId() <= ircode::OperationId::INT_LESSEQUAL ||
                binaryOp->getId() >= ircode::OperationId::FLOAT_EQUAL &&
                binaryOp->getId() <= ircode::OperationId::FLOAT_LESSEQUAL)
        {
            auto booleanDt = findDataType("bool");
            output->setDataType(booleanDt);
        }
        else if (binaryOp->getId() >= ircode::OperationId::BOOL_NEGATE &&
                binaryOp->getId() <= ircode::OperationId::BOOL_OR)
        {
            auto booleanDt = findDataType("bool");
            output->setDataType(booleanDt);
        }
        else if (binaryOp->getId() >= ircode::OperationId::FLOAT_ADD &&
                binaryOp->getId() <= ircode::OperationId::FLOAT_SQRT)
        {
            auto floatScalarDt = getScalarDataType(ScalarType::FloatingPoint, output->getSize());
            output->setDataType(floatScalarDt);
        }
    }
}

DataType* IRcodeDataTypePropagator::findDataType(const std::string& name) const {
    auto dataType = m_context->getDataTypes()->getByName(name);
    assert(dataType);
    return dataType;
}

ScalarDataType* IRcodeDataTypePropagator::getScalarDataType(ScalarType scalarType, size_t size) const {
    auto dataType = m_context->getDataTypes()->getScalar(scalarType, size);
    return dataType;
}

void IRcodeDataTypePropagator::setDefaultDataType(std::shared_ptr<ircode::Value> value) const {
    if (value->getDataType())
        return;
    auto dataType = getScalarDataType(ScalarType::UnsignedInt, value->getSize());
    value->setDataType(dataType);
}