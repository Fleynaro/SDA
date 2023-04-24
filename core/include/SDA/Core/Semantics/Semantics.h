#pragma once
#include "SDA/Core/IRcode/IRcodeProgram.h"

namespace sda::semantics
{
    struct SemanticsPropagationContext {
        ircode::Function* function = nullptr;
        const ircode::Operation* operation = nullptr;
        std::map<ircode::Function*, std::list<const ircode::Operation*>> nextOperations;

        SemanticsPropagationContext(ircode::Function* function, const ircode::Operation* operation)
            : nextOperations({ { function, { operation } } })
        {}

        void markValueAsAffected(std::shared_ptr<ircode::Value> value) {
            for (auto op : value->getOperations()) {
                nextOperations[function].push_back(op);
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

            void onOperationAddedImpl(const ircode::Operation* op, ircode::Block* block) override {
                SemanticsPropagationContext ctx(block->getFunction(), op);
                m_semManager->propagate(ctx);
            }

            void onOperationRemovedImpl(const ircode::Operation* op, ircode::Block* block) override {

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
                    ctx.function = it->first;
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
        std::map<Offset, std::list<GlobalVarSemantics*>> m_offsetToSemantics;
    public:
        GlobalVarSemanticsRepository(SemanticsManager* manager, Platform* platform)
            : BaseSemanticsRepository<GlobalVarSemantics>(manager), m_platform(platform)
        {}

        std::list<GlobalVarSemantics*> findSemanticsAtOffset(Offset offset) {
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
                        if (auto newSem = addSemantics(GlobalVarSemantics(output, sem->getSource(), sem->getOffset() + offset))) {
                            sem->addSuccessor(newSem);
                            ctx.markValueAsAffected(output);
                        }
                    }
                }
            }
        }
    };
};
