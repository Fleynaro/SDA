#pragma once
#include "Semantics.h"

namespace sda::semantics
{
    class SymbolTableSemantics : public Semantics
    {
        SymbolTable* m_symbolTable = nullptr;
        Offset m_offset = 0;
    public:
        SymbolTableSemantics() = default;

        SymbolTableSemantics(
            std::shared_ptr<ircode::Variable> variable,
            std::shared_ptr<ircode::Value> source,
            SymbolTable* symbolTable,
            Offset offset)
            : Semantics(variable, source)
            , m_symbolTable(symbolTable)
            , m_offset(offset)
        {}

        SymbolTable* getSymbolTable() const {
            return m_symbolTable;
        }

        Offset getOffset() const {
            return m_offset;
        }

        bool equals(const SymbolTableSemantics* other) const {
            return Semantics::equals(other) &&
                    m_symbolTable == other->m_symbolTable &&
                    m_offset == other->m_offset;
        }
    };

    class SymbolTableSemanticsRepository : public BaseSemanticsRepository<SymbolTableSemantics>
    {
        std::map<SymbolTable*, std::map<Offset, std::list<SymbolTableSemantics*>>> m_locToSemantics;
    public:
        SymbolTableSemanticsRepository(SemanticsManager* manager)
            : BaseSemanticsRepository<SymbolTableSemantics>(manager)
        {}

        std::list<SymbolTableSemantics*> findSemanticsAt(SymbolTable* symbolTable, Offset offset) {
            auto it = m_locToSemantics.find(symbolTable);
            if (it == m_locToSemantics.end()) {
                return std::list<SymbolTableSemantics*>();
            }
            auto it2 = it->second.find(offset);
            if (it2 == it->second.end()) {
                return std::list<SymbolTableSemantics*>();
            }
            return it2->second;
        }

        SymbolTableSemantics* addSemantics(const SymbolTableSemantics& semantics) {
            auto newSem = BaseSemanticsRepository<SymbolTableSemantics>::addSemantics(semantics);
            if (newSem) {
                auto symbolTable = semantics.getSymbolTable();
                auto offset = semantics.getOffset();
                m_locToSemantics[symbolTable][offset].push_back(newSem);
            }
            return newSem;
        }
    };

    class DataTypeSemantics : public Semantics
    {
        DataType* m_dataType = nullptr;
    public:
        DataTypeSemantics() = default;

        DataTypeSemantics(
            std::shared_ptr<ircode::Variable> variable,
            std::shared_ptr<ircode::Value> source,
            DataType* dataType)
            : Semantics(variable, source)
            , m_dataType(dataType)
        {}

        DataType* getDataType() const {
            return m_dataType;
        }

        bool equals(const DataTypeSemantics* other) const {
            return Semantics::equals(other) &&
                    m_dataType == other->m_dataType;
        }
    };

    class DataTypeSemanticsRepository : public BaseSemanticsRepository<DataTypeSemantics>
    {
    public:
        DataTypeSemanticsRepository(SemanticsManager* manager)
            : BaseSemanticsRepository<DataTypeSemantics>(manager)
        {}
    };

    class DataTypeSemanticsPropagator : public SemanticsPropagator
    {
        SymbolTable* m_globalSymbolTable = nullptr;
        SymbolTableSemanticsRepository* m_symbolTableRepo;
        DataTypeSemanticsRepository* m_dataTypeRepo;
    public:
        DataTypeSemanticsPropagator(
            SymbolTable* globalSymbolTable,
            SymbolTableSemanticsRepository* symbolTableRepo,
            DataTypeSemanticsRepository* dataTypeRepo
        )
            : m_globalSymbolTable(globalSymbolTable)
            , m_symbolTableRepo(symbolTableRepo)
            , m_dataTypeRepo(dataTypeRepo)
        {}

        void propagate(SemanticsPropagationContext& ctx) override
        {
            auto funcSymbol = getFunctionSymbol(ctx);
            auto output = ctx.operation->getOutput();

            if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(ctx.operation)) {
                auto input = unaryOp->getInput();
                if (unaryOp->getId() == ircode::OperationId::LOAD) {
                    if (auto inputReg = std::dynamic_pointer_cast<ircode::Register>(input)) {
                        auto signatureDt = funcSymbol->getSignature();
                        auto it = signatureDt->getStorages().find({
                            CallingConvention::Storage::Read,
                            inputReg->getRegister().getRegId()
                        });
                        if (it != signatureDt->getStorages().end()) {
                            // if it is a parameter
                            const auto& storageInfo = it->second;
                            auto paramIdx = storageInfo.paramIdx;
                            auto paramSymbol = signatureDt->getParameters()[paramIdx];
                            auto paramSymbolDt = paramSymbol->getDataType();
                            if (auto newDtSem = newDataTypeSemantics(
                                output,
                                output,
                                paramSymbolDt
                            )) {
                                ctx.markValueAsAffected(output);
                            }
                        } else {
                            auto regId = inputReg->getRegister().getRegId();
                            if (regId == Register::InstructionPointerId) {
                                if (m_symbolTableRepo->addSemantics(
                                    SymbolTableSemantics(
                                        output,
                                        output,
                                        m_globalSymbolTable,
                                        0))
                                ) {
                                    ctx.markValueAsAffected(output);
                                }
                            } else if (regId == Register::StackPointerId) {
                                auto stackSymbolTable = funcSymbol->getStackSymbolTable();
                                if (m_symbolTableRepo->addSemantics(
                                    SymbolTableSemantics(
                                        output,
                                        output,
                                        stackSymbolTable,
                                        0))
                                ) {
                                    ctx.markValueAsAffected(output);
                                }
                            }
                        }
                    } else if (auto inputVar = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                        for (auto dtSem : m_dataTypeRepo->findSemantics(inputVar)) {
                            if (auto pointerDt = dynamic_cast<PointerDataType*>(dtSem->getDataType())) {
                                auto pointedDt = pointerDt->getPointedType();
                                if (auto newDtSem = newDataTypeSemantics(
                                    output,
                                    dtSem->getSource(),
                                    pointedDt
                                )) {
                                    dtSem->addSuccessor(newDtSem);
                                    ctx.markValueAsAffected(output);
                                }
                            }
                        }
                        for (auto stSem : m_symbolTableRepo->findSemantics(inputVar)) {
                            auto symbolTable = stSem->getSymbolTable();
                            auto offset = stSem->getOffset();
                            auto symbol = getSymbolAt(funcSymbol, symbolTable, offset, false);
                            auto symbolDt = symbol->getDataType();
                            if (auto newDtSem = newDataTypeSemantics(
                                output,
                                stSem->getSource(),
                                symbolDt
                            )) {
                                stSem->addSuccessor(newDtSem);
                                ctx.markValueAsAffected(output);
                            }
                        }
                    }
                }
            }

            auto linearExpr = output->getLinearExpr();
            Offset offset = linearExpr.getConstTermValue();
            for (auto& term : linearExpr.getTerms()) {
                if (term.factor != 1 || term.value->getSize() != getContext()->getPlatform()->getPointerSize())
                    continue;
                if (auto termVar = std::dynamic_pointer_cast<ircode::Variable>(term.value)) {
                    for (auto sem : m_symbolTableRepo->findSemantics(termVar)) {
                        if (auto newSem = m_symbolTableRepo->addSemantics(
                            SymbolTableSemantics(
                                output,
                                sem->getSource(),
                                sem->getSymbolTable(),
                                offset))
                        ) {
                            sem->addSuccessor(newSem);
                            ctx.markValueAsAffected(output);
                        }
                    }
                }
            }
        }

    private:
        Context* getContext() const {
            return m_globalSymbolTable->getContext();
        }

        FunctionSymbol* getFunctionSymbol(SemanticsPropagationContext& ctx) const {
            auto function = ctx.operation->getBlock()->getFunction();
            auto offset = function->getEntryOffset();
            auto symbol = m_globalSymbolTable->getSymbolAt(offset).symbol;
            return dynamic_cast<FunctionSymbol*>(symbol);
        }

        DataTypeSemantics* newDataTypeSemantics(
            std::shared_ptr<ircode::Variable> value,
            std::shared_ptr<ircode::Value> source,
            DataType* dataType)
        {
            if (auto newDtSem =  m_dataTypeRepo->addSemantics(
                DataTypeSemantics(
                    value,
                    source,
                    dataType
                )
            )) {
                if (auto pointerDt = dynamic_cast<PointerDataType*>(dataType)) {
                    if (auto structDt = dynamic_cast<StructureDataType*>(pointerDt->getPointedType())) {
                        if (auto newStSem = m_symbolTableRepo->addSemantics(
                            SymbolTableSemantics(
                                newDtSem->getVariable(),
                                newDtSem->getSource(),
                                structDt->getSymbolTable(),
                                0
                            )
                        )) {
                            newDtSem->addSuccessor(newStSem);
                        }
                    }
                }
                return newDtSem;
            }
            return nullptr;
        }

        Symbol* getSymbolAt(
            FunctionSymbol* funcSymbol,
            SymbolTable* symbolTable,
            Offset offset,
            bool write) const
        {
            if (symbolTable == m_globalSymbolTable ||
                symbolTable == funcSymbol->getStackSymbolTable())
            {
                CallingConvention::Storage storage;
                storage.useType = write ? CallingConvention::Storage::Write : CallingConvention::Storage::Read;
                storage.registerId = Register::StackPointerId;
                if (symbolTable == m_globalSymbolTable)
                    storage.registerId = Register::InstructionPointerId;
                storage.offset = offset;
                auto signatureDt = funcSymbol->getSignature();
                auto it = signatureDt->getStorages().find(storage);
                if (it != signatureDt->getStorages().end()) {
                    const auto& storageInfo = it->second;
                    auto paramIdx = storageInfo.paramIdx;
                    auto paramSymbol = signatureDt->getParameters()[paramIdx];
                    return paramSymbol;
                }
            }
            return symbolTable->getSymbolAt(offset).symbol;
        }
    };
};
