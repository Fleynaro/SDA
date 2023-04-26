#pragma once
#include "SDA/Core/IRcode/IRcodeProgram.h"
#include "SDA/Core/Symbol/FunctionSymbol.h"
#include "SDA/Core/DataType/ScalarDataType.h"
#include "SDA/Core/DataType/PointerDataType.h"
#include "SDA/Core/DataType/ArrayDataType.h"
#include "SDA/Core/DataType/StructureDataType.h"

namespace sda::semantics
{
    struct SemanticsPropagationContext {
        const ircode::Operation* operation = nullptr;
        std::map<ircode::Function*, std::list<const ircode::Operation*>> nextOperations;

        void addNextOperation(const ircode::Operation* operation) {
            auto func = operation->getBlock()->getFunction();
            auto it = nextOperations.find(func);
            if (it == nextOperations.end()) {
                it = nextOperations.insert(
                    std::make_pair(func, std::list<const ircode::Operation*>())).first;
            }
            auto& ops = it->second;
            if (std::find(ops.begin(), ops.end(), operation) == ops.end()) {
                ops.push_back(operation);
            }
        }

        void markValueAsAffected(std::shared_ptr<ircode::Value> value) {
            for (auto op : value->getOperations()) {
                addNextOperation(op);
            }
        }
    };

    class SemanticsManager;
    class Semantics;
    class SemanticsRepository
    {
        SemanticsManager* m_manager;
    public:
        SemanticsRepository(SemanticsManager* manager)
            : m_manager(manager)
        {}

        SemanticsManager* getManager() const {
            return m_manager;
        }

        virtual std::list<Semantics*> findSemanticsOfVariable(std::shared_ptr<ircode::Variable> variable) = 0;

        virtual void propogate(SemanticsPropagationContext& ctx) = 0;
    };

    class SemanticsManager
    {
        std::list<std::unique_ptr<SemanticsRepository>> m_repositories;

        class IRcodeProgramCallbacks : public ircode::Program::Callbacks
        {
            SemanticsManager* m_semManager;

            void onOperationAddedImpl(const ircode::Operation* op) override {
                SemanticsPropagationContext ctx;
                ctx.addNextOperation(op);
                m_semManager->propagate(ctx);
            }

            void onOperationRemovedImpl(const ircode::Operation* op) override {

            }
        public:
            IRcodeProgramCallbacks(SemanticsManager* semManager) : m_semManager(semManager) {}
        };
        std::shared_ptr<IRcodeProgramCallbacks> m_ircodeProgramCallbacks;
    public:
        SemanticsManager(ircode::Program* program)
            : m_ircodeProgramCallbacks(std::make_shared<IRcodeProgramCallbacks>(this))
        {
            auto prevCallbacks = program->getCallbacks();
            m_ircodeProgramCallbacks->setPrevCallbacks(prevCallbacks);
            program->setCallbacks(m_ircodeProgramCallbacks);
        }

        void addRepository(std::unique_ptr<SemanticsRepository> repository) {
            m_repositories.push_back(std::move(repository));
        }

        void propagate(SemanticsPropagationContext& ctx) {
            while(!ctx.nextOperations.empty()) {
                auto it = ctx.nextOperations.begin();
                while (it != ctx.nextOperations.end()) {
                    auto& ops = it->second;
                    auto it2 = ops.begin();
                    while (it2 != ops.end()) {
                        ctx.operation = *it2;
                        for (auto& repo : m_repositories) {
                            repo->propogate(ctx);
                        }
                        it2 = ops.erase(it2);
                    }
                    it = ctx.nextOperations.erase(it);
                }
            }
        }
    };

    class Semantics
    {
        std::shared_ptr<ircode::Variable> m_variable;
        std::shared_ptr<ircode::Value> m_source;
        std::list<Semantics*> m_predecessors;
        std::list<Semantics*> m_successors;
    public:
        Semantics() = default;

        Semantics(std::shared_ptr<ircode::Variable> variable, std::shared_ptr<ircode::Value> source)
            : m_variable(variable), m_source(source)
        {}

        std::shared_ptr<ircode::Value> getSource() const {
            return m_source;
        }

        std::shared_ptr<ircode::Variable> getVariable() const {
            return m_variable;
        }

        const std::list<Semantics*>& getPredecessors() const {
            return m_predecessors;
        }

        const std::list<Semantics*>& getSuccessors() const {
            return m_successors;
        }

        void addSuccessor(Semantics* sem) {
            m_successors.push_back(sem);
            sem->m_predecessors.push_back(this);
        }

        bool equals(const Semantics* other) const {
            return m_source == other->m_source;
        }
    };

    template<typename T>
    class BaseSemanticsRepository : public SemanticsRepository
    {
        std::map<std::shared_ptr<ircode::Variable>, std::list<T>> m_items;
    public:
        using SemanticsRepository::SemanticsRepository;        

        std::list<Semantics*> findSemanticsOfVariable(std::shared_ptr<ircode::Variable> variable) override {
            std::list<Semantics*> result;
            for (auto sem : findSemantics(variable)) {
                result.push_back(sem);
            }
            return result;
        }

        std::list<T*> findSemantics(std::shared_ptr<ircode::Variable> variable) {
            auto it = m_items.find(variable);
            if (it == m_items.end()) {
                return std::list<T*>();
            }
            std::list<T*> result;
            for (auto& item : it->second) {
                result.push_back(&item);
            }
            return result;
        }

        T* addSemantics(const T& semantics) {
            auto variable = semantics.getVariable();
            for (auto sem : findSemantics(variable)) {
                if (sem->equals(&semantics)) {
                    return nullptr;
                }
            }
            m_items[variable].push_back(semantics);
            return &m_items[variable].back();
        }
    };

    class GlobalVarSemantics : public Semantics
    {
        Offset m_offset = 0;
    public:
        GlobalVarSemantics() = default;

        GlobalVarSemantics(
            std::shared_ptr<ircode::Variable> variable,
            std::shared_ptr<ircode::Value> source,
            Offset offset)
            : Semantics(variable, source)
            , m_offset(offset)
        {}

        Offset getOffset() const {
            return m_offset;
        }

        bool equals(const GlobalVarSemantics* other) const {
            return Semantics::equals(other) &&
                    m_offset == other->m_offset;
        }
    };

    class GlobalVarSemanticsRepository : public BaseSemanticsRepository<GlobalVarSemantics>
    {
        Platform* m_platform;
        std::map<Offset, std::list<GlobalVarSemantics*>> m_offsetToSemantics;
    public:
        GlobalVarSemanticsRepository(SemanticsManager* manager, Platform* platform)
            : BaseSemanticsRepository<GlobalVarSemantics>(manager), m_platform(platform)
        {}

        std::list<GlobalVarSemantics*> findSemanticsAt(Offset offset) {
            auto it = m_offsetToSemantics.find(offset);
            if (it == m_offsetToSemantics.end()) {
                return std::list<GlobalVarSemantics*>();
            }
            return it->second;
        }

        GlobalVarSemantics* addSemantics(const GlobalVarSemantics& semantics) {
            auto newSem = BaseSemanticsRepository<GlobalVarSemantics>::addSemantics(semantics);
            if (newSem) {
                auto offset = semantics.getOffset();
                m_offsetToSemantics[offset].push_back(newSem);
            }
            return newSem;
        }

        void propogate(SemanticsPropagationContext& ctx) override
        {
            auto output = ctx.operation->getOutput();

            if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(ctx.operation)) {
                auto input = unaryOp->getInput();
                if (unaryOp->getId() == ircode::OperationId::LOAD) {
                    if (auto inputReg = std::dynamic_pointer_cast<ircode::Register>(input)) {
                        if (inputReg->getRegister().getRegId() == Register::InstructionPointerId) {
                            if (addSemantics(GlobalVarSemantics(output, output, 0))) {
                                ctx.markValueAsAffected(output);
                            }
                        }
                    }
                }
            }

            auto linearExpr = output->getLinearExpr();
            Offset offset = linearExpr.getConstTermValue();
            for (auto& term : linearExpr.getTerms()) {
                if (term.factor != 1 || term.value->getSize() != m_platform->getPointerSize())
                    continue;
                if (auto termVar = std::dynamic_pointer_cast<ircode::Variable>(term.value)) {
                    for (auto sem : findSemantics(termVar)) {
                        if (auto newSem = addSemantics(GlobalVarSemantics(output, sem->getSource(), offset))) {
                            sem->addSuccessor(newSem);
                            ctx.markValueAsAffected(output);
                        }
                    }
                }
            }
        }
    };

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
        SymbolTable* m_globalSymbolTable = nullptr;
        std::map<std::pair<SymbolTable*, Offset>, std::list<SymbolTableSemantics*>> m_locToSemantics;
    public:
        SymbolTableSemanticsRepository(SemanticsManager* manager, SymbolTable* globalSymbolTable)
            : BaseSemanticsRepository<SymbolTableSemantics>(manager), m_globalSymbolTable(globalSymbolTable)
        {}

        std::list<SymbolTableSemantics*> findSemanticsAt(SymbolTable* symbolTable, Offset offset) {
            auto it = m_locToSemantics.find(std::make_pair(symbolTable, offset));
            if (it == m_locToSemantics.end()) {
                return std::list<SymbolTableSemantics*>();
            }
            return it->second;
        }

        SymbolTableSemantics* addSemantics(const SymbolTableSemantics& semantics) {
            auto newSem = BaseSemanticsRepository<SymbolTableSemantics>::addSemantics(semantics);
            if (newSem) {
                auto symbolTable = semantics.getSymbolTable();
                auto offset = semantics.getOffset();
                m_locToSemantics[std::make_pair(symbolTable, offset)].push_back(newSem);
            }
            return newSem;
        }

        void propogate(SemanticsPropagationContext& ctx) override
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
                            if (auto symbolTable = extractSymbolTable(paramSymbol)) {
                                if (addSemantics(
                                    SymbolTableSemantics(
                                        output,
                                        output,
                                        symbolTable,
                                        0))
                                ) {
                                    ctx.markValueAsAffected(output);
                                }
                            }
                        } else {
                            auto regId = inputReg->getRegister().getRegId();
                            if (regId == Register::InstructionPointerId) {
                                if (addSemantics(
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
                                if (addSemantics(
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
                        for (auto sem : findSemantics(inputVar)) {
                            auto symbolTable = sem->getSymbolTable();
                            auto offset = sem->getOffset();
                            auto symbols = getAllSymbolsAt(funcSymbol, symbolTable, offset, false);
                            for (auto& [symbolOffset, symbol] : symbols) {
                                if (auto symbolTable = extractSymbolTable(symbol)) {
                                    if (addSemantics(
                                        SymbolTableSemantics(
                                            output,
                                            sem->getSource(),
                                            symbolTable,
                                            0))
                                    ) {
                                        ctx.markValueAsAffected(output);
                                    }
                                }
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
                    for (auto sem : findSemantics(termVar)) {
                        if (auto newSem = addSemantics(
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

        SymbolTable* extractSymbolTable(Symbol* symbol) const {
            auto symbolDt = symbol->getDataType();
            if (auto pointerDt = dynamic_cast<const PointerDataType*>(symbolDt)) {
                if (auto structDt = dynamic_cast<const StructureDataType*>(pointerDt->getPointedType())) {
                    return structDt->getSymbolTable();
                }
            }
            return nullptr;
        }

        std::list<std::pair<Offset, Symbol*>> getAllSymbolsAt(
            FunctionSymbol* funcSymbol,
            SymbolTable* symbolTable,
            Offset offset,
            bool write) const
        {
            std::list<std::pair<Offset, Symbol*>> symbols;
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
    };
};
