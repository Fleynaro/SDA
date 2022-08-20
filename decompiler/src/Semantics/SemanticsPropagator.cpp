#include "Decompiler/Semantics/SemanticsPropagator.h"
#include "Decompiler/Semantics/SemanticsManager.h"
#include "Core/DataType/ScalarDataType.h"
#include "Core/DataType/PointerDataType.h"
#include "Core/DataType/ArrayDataType.h"
#include "Core/DataType/StructureDataType.h"

using namespace sda;
using namespace sda::decompiler;

SemanticsPropagator::SemanticsPropagator(SemanticsManager* semManager)
    : m_semManager(semManager)
{}

SemanticsManager* SemanticsPropagator::getManager() const {
    return m_semManager;
}

VariableSemObj* SemanticsPropagator::getOrCreateVarObject(
    const std::shared_ptr<SemanticsContext>& ctx,
    const std::shared_ptr<ircode::Variable>& var) const
{
    auto id = VariableSemObj::GetId(var.get());
    if (auto obj = getManager()->getObject<VariableSemObj>(id))
        return obj;
    auto newObj = getManager()->addObject(std::make_unique<VariableSemObj>(var.get(), ctx));
    return dynamic_cast<VariableSemObj*>(newObj);
}

void SemanticsPropagator::bindEachOther(SemanticsObject* obj1, SemanticsObject* obj2) const {
    obj1->bindTo(obj2);
    obj2->bindTo(obj1);
}

void SemanticsPropagator::markAsEffected(SemanticsObject* obj, SemanticsContextOperations& nextOps) const {
    obj->getAllRelatedOperations(nextOps);
}

bool SemanticsPropagator::checkSemantics(
    const SemanticsObject* obj,
    Semantics::FilterFunction filter,
    Semantics* predSem) const
{
    if (!getManager()->isSimiliarityConsidered()) {
        if (predSem) {
            filter = [predSem](const Semantics* sem) {
                auto& preds = sem->getPredecessors();
                return std::find(preds.begin(), preds.end(), predSem) != preds.end();
            };
        } else {
            filter = Semantics::FilterAnd(filter, [obj](const Semantics* sem) {
                return sem->isSource(Semantics::System);
            });
        }
    }
    return obj->checkSemantics(filter);
}

void SemanticsPropagator::propagateTo(
    SemanticsObject* fromObj,
    SemanticsObject* toObj,
    Semantics::FilterFunction filter,
    SemanticsContextOperations& nextOps,
    size_t uncertaintyDegree) const
{
    if (getManager()->isSimiliarityConsidered()) {
        auto toSemantics = toObj->findSemantics(filter);
        filter = Semantics::FilterAnd(filter, [toSemantics](const Semantics* fromSem) {
            for (auto toSem : toSemantics) {
                if (fromSem->isSimiliarTo(toSem))
                    return false;
            }
            return true;
        });
    }

    filter = Semantics::FilterAnd(filter, [toObj](const Semantics* fromSem) {
        for (auto succ : fromSem->getSuccessors()) {
            if (succ->getHolder() == toObj)
                return false;
        }
        return true;
    });

    auto fromSemantics = fromObj->findSemantics(filter);
    for (auto fromSem : fromSemantics) {
        auto newMetaInfo = fromSem->getMetaInfo();
        newMetaInfo.uncertaintyDegree = newMetaInfo.uncertaintyDegree + uncertaintyDegree;
        auto toSem = getManager()->addSemantics(fromSem->clone(toObj, newMetaInfo));
        fromSem->addSuccessor(toSem);
    }
    
    if (!fromSemantics.empty())
        markAsEffected(toObj, nextOps);
}

void BaseSemanticsPropagator::propagate(
    const std::shared_ptr<SemanticsContext>& ctx,
    const ircode::Operation* op,
    SemanticsContextOperations& nextOps)
{
    auto output = op->getOutput();

    if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(op)) {
        auto input = unaryOp->getInput();
        if (unaryOp->getId() == ircode::OperationId::LOAD) {
            if (auto inputReg = std::dynamic_pointer_cast<ircode::Register>(input)) {
                auto signatureDt = ctx->functionSymbol->getSignature();
                auto it = signatureDt->getStorages().find({
                    CallingConvention::Storage::Read,
                    inputReg->getRegister().getRegId()
                });
                if (it != signatureDt->getStorages().end()) {
                    // if it is a parameter
                    const auto& storageInfo = it->second;
                    auto paramIdx = storageInfo.paramIdx;
                    auto paramSymbol = signatureDt->getParameters()[paramIdx];
                    if (auto paramSymbolObj = getSymbolObject(paramSymbol)) {
                        auto outputVarObj = getOrCreateVarObject(ctx, output);
                        bindEachOther(paramSymbolObj, outputVarObj);
                        propagateTo(paramSymbolObj, outputVarObj, DataTypeSemantics::Filter(), nextOps);
                        propagateTo(outputVarObj, paramSymbolObj, DataTypeSemantics::Filter(), nextOps);
                    }
                } else {
                    // if it is a stack or global variable
                    SymbolTable* symbolTable = nullptr;
                    if (inputReg->getRegister().getRegId() == Register::StackPointerId)
                        symbolTable = ctx->functionSymbol->getStackSymbolTable();
                    else if (inputReg->getRegister().getRegId() == Register::InstructionPointerId)
                        symbolTable = ctx->globalSymbolTable;
                    if (symbolTable) {
                        if (auto symbolTableObj = getSymbolTableObject(symbolTable)) {
                            auto outputVarObj = getOrCreateVarObject(ctx, output);
                            bindEachOther(symbolTableObj, outputVarObj);
                            propagateTo(symbolTableObj, outputVarObj, SymbolTableSemantics::Filter(), nextOps);
                        }
                    }
                }
            }
            else if (auto inputVar = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                auto inputVarObj = getOrCreateVarObject(ctx, inputVar);
                auto outputVarObj = getOrCreateVarObject(ctx, output);
                auto loadSize = output->getSize();

                auto linearExpr = inputVar->getLinearExpr();
                Offset offset = linearExpr.getConstTermValue();
                auto pointers = getAllPointers(ctx, linearExpr);
                for (auto& ptr : pointers) {
                    if (ptr.symbolTable) {
                        const auto symbols = getAllSymbolsAt(ctx, ptr.symbolTable, offset);
                        for (auto& [symbolOffset, symbol] : symbols) {
                            auto symbolDt = symbol->getDataType();
                            if (loadSize <= symbolDt->getSize()) {
                                if (auto symbolObj = getSymbolObject(symbol)) {
                                    bindEachOther(symbolObj, outputVarObj);       
                                    auto semantics = symbolObj->findSemantics(DataTypeSemantics::Filter());
                                    for (auto sem : semantics) {
                                        if (auto dataTypeSem = dynamic_cast<DataTypeSemantics*>(sem)) {
                                            auto symbolDt = dataTypeSem->getDataType();
                                            if (!checkSemantics(outputVarObj, DataTypeSemantics::Filter(symbolDt), sem)) {
                                                DataTypeSemantics::SliceInfo sliceInfo = {};
                                                auto relOffset = offset - symbolOffset;
                                                if (relOffset != 0 || loadSize != symbolDt->getSize()) {
                                                    sliceInfo.type = DataTypeSemantics::SliceInfo::Load;
                                                    sliceInfo.offset = relOffset;
                                                    sliceInfo.size = loadSize;
                                                }
                                                auto newSem = createDataTypeSemantics(outputVarObj, sem->getSourceInfo(), symbolDt, sliceInfo, sem->getMetaInfo());
                                                sem->addSuccessor(newSem);
                                                markAsEffected(outputVarObj, nextOps);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // if loading value at address that points to a scalar or array of scalars (not structures)
                    if (ptr.dataType) {
                        DataType* itemDt = nullptr;
                        if (auto pointerDt = dynamic_cast<PointerDataType*>(ptr.dataType)) {
                            itemDt = pointerDt->getPointedType();
                        } else if (auto arrayDt = dynamic_cast<ArrayDataType*>(ptr.dataType)) {
                            itemDt = arrayDt->getElementType();
                            if (offset >= arrayDt->getSize())
                                continue;
                        }
                        auto sem = ptr.semantics;
                        if (!checkSemantics(outputVarObj, DataTypeSemantics::Filter(itemDt), sem)) {
                            DataTypeSemantics::SliceInfo sliceInfo = {};
                            auto relOffset = offset % itemDt->getSize();
                            if (relOffset != 0 || loadSize != itemDt->getSize()) {
                                sliceInfo.type = DataTypeSemantics::SliceInfo::Load;
                                sliceInfo.offset = relOffset;
                                sliceInfo.size = loadSize;
                            }
                            auto newSem = createDataTypeSemantics(outputVarObj, sem->getSourceInfo(), itemDt, sliceInfo, sem->getMetaInfo());
                            sem->addSuccessor(newSem);
                            markAsEffected(outputVarObj, nextOps);
                        }
                    }
                }
            }
        }
        else if (unaryOp->getId() == ircode::OperationId::COPY) {
            if (auto inputVar = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                auto inputVarObj = getOrCreateVarObject(ctx, inputVar);
                auto outputVarObj = getOrCreateVarObject(ctx, output);
                propagateTo(inputVarObj, outputVarObj, Semantics::FilterAll(), nextOps);
                propagateTo(outputVarObj, inputVarObj, Semantics::FilterAll(), nextOps);
            }
        }
        else if (unaryOp->getId() == ircode::OperationId::INT_2COMP) {
            auto signedScalarDt = getScalarDataType(ScalarType::SignedInt, output->getSize());
            setDataTypeFor(ctx, input, signedScalarDt, nextOps);
            setDataTypeFor(ctx, output, signedScalarDt, nextOps);
        }
        else if (unaryOp->getId() == ircode::OperationId::BOOL_NEGATE) {
            auto booleanDt = findDataType("bool");
            setDataTypeFor(ctx, input, booleanDt, nextOps);
            setDataTypeFor(ctx, output, booleanDt, nextOps);
        }
        else if (unaryOp->getId() == ircode::OperationId::FLOAT_NEG ||
                 unaryOp->getId() == ircode::OperationId::FLOAT_ABS ||
                 unaryOp->getId() == ircode::OperationId::FLOAT_SQRT) {
            auto floatScalarDt = getScalarDataType(ScalarType::FloatingPoint, output->getSize());
            setDataTypeFor(ctx, input, floatScalarDt, nextOps);
            setDataTypeFor(ctx, output, floatScalarDt, nextOps);
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
            auto outputVarObj = getOrCreateVarObject(ctx, output);
            for (auto& input : {input1, input2}) {
                if (auto inputVar = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                    auto inputVarObj = getOrCreateVarObject(ctx, inputVar);
                    auto signedScalarDt = getScalarDataType(ScalarType::SignedInt, output->getSize());
                    auto filter = DataTypeSemantics::Filter(signedScalarDt);
                    propagateTo(inputVarObj, outputVarObj, filter, nextOps);
                    propagateTo(outputVarObj, inputVarObj, filter, nextOps, 1);
                }
            }

            if (binaryOp->getId() == ircode::OperationId::INT_ADD ||
                binaryOp->getId() == ircode::OperationId::INT_MULT)
            {
                auto linearExpr = output->getLinearExpr();
                Offset offset = linearExpr.getConstTermValue();
                auto pointers = getAllPointers(ctx, linearExpr);
                auto createVoidPtrDtSem = !pointers.empty();
                for (auto& ptr : pointers) {
                    if (ptr.symbolTable) {
                        const auto symbols = getAllSymbolsAt(ctx, ptr.symbolTable, offset);
                        for (auto& [symbolOffset, symbol] : symbols) {
                            if (symbolOffset != offset)
                                continue;
                            if (auto symbolObj = getSymbolObject(symbol)) {
                                bindEachOther(symbolObj, outputVarObj);
                                auto semantics = symbolObj->findSemantics(DataTypeSemantics::Filter());
                                for (auto sem : semantics) {
                                    if (auto dataTypeSem = dynamic_cast<DataTypeSemantics*>(sem)) {
                                        auto pointerDt = dataTypeSem->getDataType()->getPointerTo();
                                        if (!checkSemantics(outputVarObj, DataTypeSemantics::Filter(pointerDt), sem)) {
                                            auto newSem = createDataTypeSemantics(outputVarObj, sem->getSourceInfo(), pointerDt, {}, sem->getMetaInfo());
                                            sem->addSuccessor(newSem);
                                            markAsEffected(outputVarObj, nextOps);
                                        }
                                        createVoidPtrDtSem = false;
                                    }
                                }
                            }
                        }
                    }

                    if (ptr.dataType) {
                        auto sem = ptr.semantics;
                        if (!checkSemantics(outputVarObj, DataTypeSemantics::Filter(ptr.dataType), sem)) {
                            DataType* itemDt = nullptr;
                            if (auto pointerDt = dynamic_cast<PointerDataType*>(ptr.dataType)) {
                                itemDt = pointerDt->getPointedType();
                            } else if (auto arrayDt = dynamic_cast<ArrayDataType*>(ptr.dataType)) {
                                itemDt = arrayDt->getElementType();
                                if (offset >= arrayDt->getSize())
                                    continue;
                            }

                            assert(itemDt);
                            if (offset % itemDt->getSize() == 0) {
                                auto newSem = createDataTypeSemantics(outputVarObj, sem->getSourceInfo(), ptr.dataType, {}, sem->getMetaInfo());
                                sem->addSuccessor(newSem);
                                markAsEffected(outputVarObj, nextOps);
                            }
                        }
                    }
                }

                if (createVoidPtrDtSem) {
                    auto voidDt = findDataType("void");
                    if (!checkSemantics(outputVarObj, DataTypeSemantics::Filter(voidDt))) {
                        auto sourceInfo = std::make_shared<Semantics::SourceInfo>();
                        sourceInfo->creatorType = Semantics::System;
                        Semantics::MetaInfo metaInfo = {};
                        for (auto& ptr : pointers) {
                            metaInfo.uncertaintyDegree = std::min(
                                metaInfo.uncertaintyDegree, ptr.semantics->getMetaInfo().uncertaintyDegree);
                        }
                        auto newSem = createDataTypeSemantics(outputVarObj, sourceInfo, voidDt->getPointerTo(), {}, metaInfo);
                        for (auto& ptr : pointers)
                            ptr.semantics->addSuccessor(newSem);
                        markAsEffected(outputVarObj, nextOps);
                    }
                }
            }
        }
        else if (binaryOp->getId() == ircode::OperationId::INT_SDIV ||
                binaryOp->getId() == ircode::OperationId::INT_SREM)
        {
            auto signedScalarDt = getScalarDataType(ScalarType::SignedInt, output->getSize());
            setDataTypeFor(ctx, output, signedScalarDt, nextOps);
        }
        else if (binaryOp->getId() >= ircode::OperationId::INT_EQUAL &&
                binaryOp->getId() <= ircode::OperationId::INT_LESSEQUAL ||
                binaryOp->getId() >= ircode::OperationId::FLOAT_EQUAL &&
                binaryOp->getId() <= ircode::OperationId::FLOAT_LESSEQUAL)
        {
            auto booleanDt = findDataType("bool");
            setDataTypeFor(ctx, output, booleanDt, nextOps);
            if (binaryOp->getId() >= ircode::OperationId::FLOAT_EQUAL &&
                binaryOp->getId() <= ircode::OperationId::FLOAT_LESSEQUAL)
                {
                    auto floatScalarDt = getScalarDataType(ScalarType::FloatingPoint, output->getSize());
                    setDataTypeFor(ctx, input1, floatScalarDt, nextOps);
                    setDataTypeFor(ctx, input2, floatScalarDt, nextOps);
                }
        }
        else if (binaryOp->getId() >= ircode::OperationId::BOOL_NEGATE &&
                binaryOp->getId() <= ircode::OperationId::BOOL_OR)
        {
            auto booleanDt = findDataType("bool");
            setDataTypeFor(ctx, input1, booleanDt, nextOps);
            setDataTypeFor(ctx, input2, booleanDt, nextOps);
            setDataTypeFor(ctx, output, booleanDt, nextOps);
        }
        else if (binaryOp->getId() >= ircode::OperationId::FLOAT_ADD &&
                binaryOp->getId() <= ircode::OperationId::FLOAT_SQRT)
        {
            auto floatScalarDt = getScalarDataType(ScalarType::FloatingPoint, output->getSize());
            setDataTypeFor(ctx, input1, floatScalarDt, nextOps);
            setDataTypeFor(ctx, input2, floatScalarDt, nextOps);
            setDataTypeFor(ctx, output, floatScalarDt, nextOps);
        }
    }

    auto outputAddrVal = output->getMemAddress().value;
    if (auto outputAddrReg = std::dynamic_pointer_cast<ircode::Register>(outputAddrVal)) {
        auto signatureDt = ctx->functionSymbol->getSignature();
        auto it = signatureDt->getStorages().find({
            CallingConvention::Storage::Write,
            outputAddrReg->getRegister().getRegId()
        });
        if (it != signatureDt->getStorages().end()) {
            if (auto funcReturnObj = getFuncReturnObject(signatureDt)) {
                auto outputVarObj = getOrCreateVarObject(ctx, output);
                bindEachOther(funcReturnObj, outputVarObj);
                propagateTo(funcReturnObj, outputVarObj, DataTypeSemantics::Filter(), nextOps);
                propagateTo(outputVarObj, funcReturnObj, DataTypeSemantics::Filter(), nextOps);
            }
        }
    }
    else if (auto outputAddrVar = std::dynamic_pointer_cast<ircode::Variable>(outputAddrVal)) {
        auto outputVarObj = getOrCreateVarObject(ctx, output);
        auto linearExpr = outputAddrVar->getLinearExpr();
        Offset offset = linearExpr.getConstTermValue();
        auto pointers = getAllPointers(ctx, linearExpr);
        for (auto& ptr : pointers) {
            const auto symbols = getAllSymbolsAt(ctx, ptr.symbolTable, offset, true);
            for (auto& [symbolOffset, symbol] : symbols) {
                if (symbolOffset != offset)
                    continue;
                if (auto symbolObj = getSymbolObject(symbol)) {
                    bindEachOther(symbolObj, outputVarObj);
                    propagateTo(symbolObj, outputVarObj, DataTypeSemantics::Filter(), nextOps);
                    propagateTo(outputVarObj, symbolObj, DataTypeSemantics::Filter(), nextOps);
                }
            }
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

DataTypeSemantics* BaseSemanticsPropagator::createDataTypeSemantics(
    SemanticsObject* holder,
    const std::shared_ptr<Semantics::SourceInfo>& sourceInfo,
    DataType* dataType,
    const DataTypeSemantics::SliceInfo& sliceInfo,
    const Semantics::MetaInfo& metaInfo) const
{
    auto sem = getManager()->addSemantics(std::make_unique<DataTypeSemantics>(
        holder,
        sourceInfo,
        dataType,
        sliceInfo,
        metaInfo));
    return dynamic_cast<DataTypeSemantics*>(sem);
}

void BaseSemanticsPropagator::setDataTypeFor(
    const std::shared_ptr<SemanticsContext>& ctx,
    std::shared_ptr<ircode::Value> value,
    DataType* dataType,
    SemanticsContextOperations& nextOps) const
{
    if (auto var = std::dynamic_pointer_cast<ircode::Variable>(value)) {
        auto varObj = getOrCreateVarObject(ctx, var);
        if (!checkSemantics(varObj, DataTypeSemantics::Filter(dataType))) { // ?
            auto sourceInfo = std::make_shared<Semantics::SourceInfo>();
            sourceInfo->creatorType = Semantics::System;
            createDataTypeSemantics(varObj, sourceInfo, dataType);
            markAsEffected(varObj, nextOps);
        }
    }
}

std::list<BaseSemanticsPropagator::PointerInfo> BaseSemanticsPropagator::getAllPointers(
    const std::shared_ptr<SemanticsContext>& ctx,
    const ircode::LinearExpression& linearExpr) const
{
    std::list<PointerInfo> result;
    for (auto& term : linearExpr.getTerms()) {
        if (!term.canBePointer())
            continue;
        if (auto termVar = std::dynamic_pointer_cast<ircode::Variable>(term.value)) {
            auto termVarObj = getOrCreateVarObject(ctx, termVar);
            auto filter = Semantics::FilterOr(DataTypeSemantics::Filter(), SymbolTableSemantics::Filter());
            auto semantics = termVarObj->findSemantics(filter);
            for (auto sem : semantics) {
                PointerInfo info = {};
                if (auto symbolTableSem = dynamic_cast<SymbolTableSemantics*>(sem)) {
                    info.symbolTable = symbolTableSem->getSymbolTable();
                    result.push_back(info);
                } else if (auto dataTypeSem = dynamic_cast<DataTypeSemantics*>(sem)) {
                    if (auto pointerDt = dynamic_cast<const PointerDataType*>(dataTypeSem->getDataType())) {
                        if (auto structDt = dynamic_cast<const StructureDataType*>(pointerDt->getPointedType()))
                            info.symbolTable = structDt->getSymbolTable();
                        info.dataType = dataTypeSem->getDataType();
                        result.push_back(info);
                    }
                    else if (dynamic_cast<const ArrayDataType*>(dataTypeSem->getDataType())) {
                        // "array" is just a pointer to repeated data elements but with max size constraint
                        info.dataType = dataTypeSem->getDataType();
                        result.push_back(info);
                    }
                }
            }
        }
    }
    return result;
}

std::list<std::pair<Offset, Symbol*>> BaseSemanticsPropagator::getAllSymbolsAt(
    const std::shared_ptr<SemanticsContext>& ctx,
    SymbolTable* symbolTable,
    Offset offset,
    bool write) const
{
    std::list<std::pair<Offset, Symbol*>> symbols;
    if (symbolTable == ctx->globalSymbolTable ||
        symbolTable == ctx->functionSymbol->getStackSymbolTable())
    {
        CallingConvention::Storage storage;
        storage.useType = write ? CallingConvention::Storage::Write : CallingConvention::Storage::Read;
        storage.registerId = Register::StackPointerId;
        if (symbolTable == ctx->globalSymbolTable)
            storage.registerId = Register::InstructionPointerId;
        storage.offset = offset;
        auto signatureDt = ctx->functionSymbol->getSignature();
        auto it = signatureDt->getStorages().find(storage);
        if (it != signatureDt->getStorages().end()) {
            const auto& storageInfo = it->second;
            auto paramIdx = storageInfo.paramIdx;
            auto paramSymbol = signatureDt->getParameters()[paramIdx];
            symbols.emplace_back(offset, paramSymbol);
        }
    }
    if (symbols.empty()) {
        auto foundSymbols = symbolTable->getAllSymbolsRecursivelyAt(offset);
        for (auto [_, symbolOffset, symbol] : foundSymbols)
            symbols.emplace_back(symbolOffset, symbol);
    }
    return symbols;
}