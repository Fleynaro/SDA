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

bool SemanticsPropagator::checkSemantics(
    const SemanticsObject* obj,
    Semantics::FilterFunction filter,
    Semantics* predSem) const
{
    bool onlyEmitted;
    if (getManager()->isSimiliarityConsidered()) {
        onlyEmitted = false;
    } else {
        if (predSem) {
            filter = [predSem](const Semantics* sem) {
                auto preds = sem->getPredecessors();
                return std::find(preds.begin(), preds.end(), predSem) == preds.end();
            };
        }
        onlyEmitted = true;
    }
    return !obj->checkSemantics(filter, onlyEmitted);
}

void SemanticsPropagator::propagateTo(
    SemanticsObject* fromObj,
    SemanticsObject* toObj,
    Semantics::FilterFunction filter,
    std::set<SemanticsObject*>& nextObjs,
    size_t uncertaintyDegree) const
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

    bool isPropagated = false;
    auto fromObjSemantics = fromObj->findSemantics(filter);
    for (auto sem : fromObjSemantics) {
        if (uncertaintyDegree > 0) {
            auto newMetaInfo = sem->getMetaInfo();
            newMetaInfo.uncertaintyDegree = newMetaInfo.uncertaintyDegree + uncertaintyDegree;
            auto newSem = getManager()->addSemantics(sem->clone(newMetaInfo));
            sem->addSuccessor(newSem);
            isPropagated |= toObj->addSemantics(newSem, true);
        } else {
            isPropagated |= toObj->addSemantics(sem);
        }
    }

    if (isPropagated)
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
                        auto outputVarObj = getOrCreateVarObject(output);
                        bindEachOther(paramSymbolObj, outputVarObj);
                        propagateTo(paramSymbolObj, outputVarObj, DataTypeSemantics::Filter(), nextObjs);
                        propagateTo(outputVarObj, paramSymbolObj, DataTypeSemantics::Filter(), nextObjs);
                    }
                } else {
                    // if it is a stack or global variable
                    auto symbolTable = ctx->functionSymbol->getStackSymbolTable();
                    if (inputReg->getRegister().getRegId() == Register::InstructionPointerId)
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
            else if (auto inputVar = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                auto inputVarObj = getOrCreateVarObject(inputVar);
                auto outputVarObj = getOrCreateVarObject(output);
                auto loadSize = output->getSize();

                auto linearExpr = inputVar->getLinearExpr();
                Offset offset = linearExpr.getConstTermValue();
                auto pointers = getAllPointers(linearExpr);
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
                                                    auto newSem = createDataTypeSemantics(sem->getSourceInfo(), symbolDt, sliceInfo, sem->getMetaInfo());
                                                    if (outputVarObj->addSemantics(newSem, true))
                                                        nextObjs.insert(outputVarObj);
                                                    sem->addSuccessor(newSem);
                                                } else {
                                                    if (outputVarObj->addSemantics(sem))
                                                        nextObjs.insert(outputVarObj);
                                                }
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
                            auto newSem = createDataTypeSemantics(sem->getSourceInfo(), itemDt, sliceInfo, sem->getMetaInfo());
                            if (outputVarObj->addSemantics(newSem, true))
                                nextObjs.insert(outputVarObj);
                            sem->addSuccessor(newSem);
                        }
                    }
                }
            }
        }
        else if (unaryOp->getId() == ircode::OperationId::COPY) {
            if (auto inputVar = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                auto inputVarObj = getOrCreateVarObject(inputVar);
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
            for (auto& input : {input1, input2}) {
                if (auto inputVar = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                    auto inputVarObj = getOrCreateVarObject(inputVar);
                    auto signedScalarDt = getScalarDataType(ScalarType::SignedInt, output->getSize());
                    auto filter = DataTypeSemantics::Filter(signedScalarDt);
                    propagateTo(inputVarObj, outputVarObj, filter, nextObjs);
                    propagateTo(outputVarObj, inputVarObj, filter, nextObjs, 1);
                }
            }

            if (binaryOp->getId() == ircode::OperationId::INT_ADD ||
                binaryOp->getId() == ircode::OperationId::INT_MULT)
            {
                auto linearExpr = output->getLinearExpr();
                Offset offset = linearExpr.getConstTermValue();
                auto pointers = getAllPointers(linearExpr);
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
                                            auto newSem = createDataTypeSemantics(sem->getSourceInfo(), pointerDt, {}, sem->getMetaInfo());
                                            if (outputVarObj->addSemantics(newSem, true))
                                                nextObjs.insert(outputVarObj);
                                            sem->addSuccessor(newSem);
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
                                auto newSem = createDataTypeSemantics(sem->getSourceInfo(), ptr.dataType, {}, sem->getMetaInfo());
                                if (outputVarObj->addSemantics(newSem, true))
                                    nextObjs.insert(outputVarObj);
                                sem->addSuccessor(newSem);
                            }
                        }
                    }
                }

                if (createVoidPtrDtSem) {
                    auto voidDt = findDataType("void");
                    if (!checkSemantics(outputVarObj, DataTypeSemantics::Filter(voidDt))) {
                        auto sourceInfo = std::make_shared<Semantics::SourceInfo>();
                        sourceInfo->creatorType = Semantics::SourceInfo::System;
                        Semantics::MetaInfo metaInfo = {};
                        for (auto& ptr : pointers) {
                            metaInfo.uncertaintyDegree = std::min(
                                metaInfo.uncertaintyDegree, ptr.semantics->getMetaInfo().uncertaintyDegree);
                        }
                        auto newSem = createDataTypeSemantics(sourceInfo, voidDt->getPointerTo(), {}, metaInfo);
                        if (outputVarObj->addSemantics(newSem, true))
                            nextObjs.insert(outputVarObj);
                        for (auto& ptr : pointers) {
                            ptr.semantics->addSuccessor(newSem);
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

    auto outputAddrVal = output->getMemAddress().value;
    if (auto outputAddrReg = std::dynamic_pointer_cast<ircode::Register>(outputAddrVal)) {
        auto signatureDt = ctx->functionSymbol->getSignature();
        auto it = signatureDt->getStorages().find({
            CallingConvention::Storage::Write,
            outputAddrReg->getRegister().getRegId()
        });
        if (it != signatureDt->getStorages().end()) {
            if (auto funcReturnObj = getFuncReturnObject(signatureDt)) {
                auto outputVarObj = getOrCreateVarObject(output);
                bindEachOther(funcReturnObj, outputVarObj);
                propagateTo(funcReturnObj, outputVarObj, DataTypeSemantics::Filter(), nextObjs);
                propagateTo(outputVarObj, funcReturnObj, DataTypeSemantics::Filter(), nextObjs);
            }
        }
    }
    else if (auto outputAddrVar = std::dynamic_pointer_cast<ircode::Variable>(outputAddrVal)) {
        auto outputVarObj = getOrCreateVarObject(output);
        auto linearExpr = outputAddrVar->getLinearExpr();
        Offset offset = linearExpr.getConstTermValue();
        auto pointers = getAllPointers(linearExpr);
        for (auto& ptr : pointers) {
            const auto symbols = getAllSymbolsAt(ctx, ptr.symbolTable, offset, true);
            for (auto& [symbolOffset, symbol] : symbols) {
                if (symbolOffset != offset)
                    continue;
                if (auto symbolObj = getSymbolObject(symbol)) {
                    bindEachOther(symbolObj, outputVarObj);
                    propagateTo(symbolObj, outputVarObj, DataTypeSemantics::Filter(), nextObjs);
                    propagateTo(outputVarObj, symbolObj, DataTypeSemantics::Filter(), nextObjs);
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
    const std::shared_ptr<Semantics::SourceInfo>& sourceInfo,
    DataType* dataType,
    const DataTypeSemantics::SliceInfo& sliceInfo,
    const Semantics::MetaInfo& metaInfo) const
{
    auto sem = getManager()->addSemantics(std::make_unique<DataTypeSemantics>(
        sourceInfo,
        dataType,
        sliceInfo,
        metaInfo));
    return dynamic_cast<DataTypeSemantics*>(sem);
}

void BaseSemanticsPropagator::setDataTypeFor(std::shared_ptr<ircode::Value> value, DataType* dataType, std::set<SemanticsObject*>& nextObjs) const {
    if (auto var = std::dynamic_pointer_cast<ircode::Variable>(value)) {
        auto varObj = getOrCreateVarObject(var);
        if (!checkSemantics(varObj, DataTypeSemantics::Filter(dataType))) {
            auto sourceInfo = std::make_shared<Semantics::SourceInfo>();
            sourceInfo->creatorType = Semantics::SourceInfo::System;
            auto sem = createDataTypeSemantics(sourceInfo, dataType);
            varObj->addSemantics(sem, true);
            nextObjs.insert(varObj);
        }
    }
}

std::list<BaseSemanticsPropagator::PointerInfo> BaseSemanticsPropagator::getAllPointers(const ircode::LinearExpression& linearExpr) const {
    std::list<PointerInfo> result;
    for (auto& term : linearExpr.getTerms()) {
        if (!term.canBePointer())
            continue;
        if (auto termVar = std::dynamic_pointer_cast<ircode::Variable>(term.value)) {
            auto termVarObj = getOrCreateVarObject(termVar);
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

std::list<std::pair<Offset, Symbol*>> BaseSemanticsPropagator::getAllSymbolsAt(const SemanticsContext* ctx, SymbolTable* symbolTable, Offset offset, bool write) const {
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