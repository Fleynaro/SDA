#pragma once
#include "SDA/Core/IRcode/IRcodeProgram.h"
#include "SDA/Core/SymbolTable/SymbolTable.h"
#include "SDA/Core/Symbol/FunctionSymbol.h"

namespace sda::semantics
{
    struct SemanticsPropagationContext {
        SymbolTable* globalSymbolTable;
        FunctionSymbol* functionSymbol;
    };

    struct SemanticsOperation {
        std::shared_ptr<SemanticsPropagationContext> context;
        const ircode::Operation* operation;

        bool operator<(const SemanticsOperation& other) const {
            return operation < other.operation;
        }
    };

    using SemanticsOperations = std::set<SemanticsOperation>;

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

        virtual void propogate(
            const std::shared_ptr<SemanticsPropagationContext>& ctx,
            const ircode::Operation* op,
            SemanticsOperations& nextOps) = 0;
    };

    class SemanticsManager
    {
        std::list<std::unique_ptr<SemanticsRepository>> m_repositories;

        class IRcodeProgramCallbacks : public ircode::Program::Callbacks
        {
            SemanticsManager* m_semManager;

            void onOperationAddedImpl(const ircode::Operation* op, ircode::Block* block) override {
                auto ctx = std::make_shared<SemanticsPropagationContext>(SemanticsPropagationContext { nullptr, nullptr });
                m_semManager->propagate(SemanticsOperations({{ ctx, op }}));
            }

            void onOperationRemovedImpl(const ircode::Operation* op, ircode::Block* block) override {

            }
        public:
            IRcodeProgramCallbacks(SemanticsManager* semManager) : m_semManager(semManager) {}
        };
        std::shared_ptr<IRcodeProgramCallbacks> m_ircodeProgramCallbacks;
    public:
        SemanticsManager() {

        }

        void addRepository(std::unique_ptr<SemanticsRepository> repository) {
            m_repositories.push_back(std::move(repository));
        }

        void propagate(SemanticsOperations& operations) {
            auto selectedOps = operations;
            while (!selectedOps.empty()) {
                auto it = selectedOps.begin();
                auto op = *it;

                for (auto& repo : m_repositories)
                    repo->propogate(op.context, op.operation, operations);

                selectedOps.erase(it);
                operations.erase(op);
            }
        }

        void propagateThroughly(SemanticsOperations& operations) {
            while (!operations.empty()) {
                propagate(operations);
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

        T* addSemantics(std::shared_ptr<ircode::Variable> variable, const T& semantics) {
            for (auto sem : findSemantics(variable)) {
                if (sem->equals(&semantics)) {
                    return nullptr;
                }
            }
            m_items[variable].push_back(semantics);
            return &m_items[variable].back();
        }

        void addValueOperations(
            const std::shared_ptr<SemanticsPropagationContext>& ctx,
            std::shared_ptr<ircode::Value> value,
            SemanticsOperations& nextOps) {
            for (auto op : value->getOperations()) {
                nextOps.insert({ ctx, op });
            }
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
            : Semantics(variable, source), m_offset(offset)
        {}

        Offset getOffset() const {
            return m_offset;
        }

        bool equals(const GlobalVarSemantics* other) const {
            return Semantics::equals(other) && m_offset == other->m_offset;
        }
    };

    class GlobalVarSemanticsRepository : public BaseSemanticsRepository<GlobalVarSemantics>
    {
        Platform* m_platform;
    public:
        GlobalVarSemanticsRepository(SemanticsManager* manager, Platform* platform)
            : BaseSemanticsRepository<GlobalVarSemantics>(manager), m_platform(platform)
        {}

        void propogate(
            const std::shared_ptr<SemanticsPropagationContext>& ctx,
            const ircode::Operation* op,
            SemanticsOperations& nextOps) override
        {
            auto output = op->getOutput();

            if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(op)) {
                auto input = unaryOp->getInput();
                if (unaryOp->getId() == ircode::OperationId::LOAD) {
                    if (auto inputReg = std::dynamic_pointer_cast<ircode::Register>(input)) {
                        if (inputReg->getRegister().getRegId() == Register::InstructionPointerId) {
                            if (addSemantics(output, GlobalVarSemantics(output, output, 0))) {
                                addValueOperations(ctx, output, nextOps);
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
                        if (auto newSem = addSemantics(output, GlobalVarSemantics(termVar, sem->getSource(), sem->getOffset() + offset))) {
                            sem->addSuccessor(newSem);
                            addValueOperations(ctx, termVar, nextOps);
                        }
                    }
                }
            }
        }
    };
};
