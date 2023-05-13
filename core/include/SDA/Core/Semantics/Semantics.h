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
    };

    class SemanticsPropagator
    {
    public:
        virtual void propagate(SemanticsPropagationContext& ctx) = 0;
    };

    class SemanticsManager
    {
        std::list<std::unique_ptr<SemanticsRepository>> m_repositories;
        std::list<std::unique_ptr<SemanticsPropagator>> m_propagators;

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

        void addPropagator(std::unique_ptr<SemanticsPropagator> propagator) {
            m_propagators.push_back(std::move(propagator));
        }

        void propagate(SemanticsPropagationContext ctx) {
            for (auto& propagator : m_propagators) {
                propagate(ctx, propagator.get());
            }
        }

        void propagate(SemanticsPropagationContext ctx, SemanticsPropagator* propagator) {
            while(!ctx.nextOperations.empty()) {
                auto it = ctx.nextOperations.begin();
                while (it != ctx.nextOperations.end()) {
                    auto& ops = it->second;
                    auto it2 = ops.begin();
                    while (it2 != ops.end()) {
                        ctx.operation = *it2;
                        propagator->propagate(ctx);
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
};
