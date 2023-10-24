#pragma once
#include "ResearcherHelper.h"
#include "DataFlowResearcher.h"
#include "SDA/Core/IRcode/IRcodeBlock.h"
#include "SDA/Core/IRcode/IRcodeEvents.h"
#include "SDA/Core/Utils/Logger.h"
#include "SDA/Core/Utils/String.h"

namespace sda::researcher
{
    static const size_t SemanticsEventTopic = TopicName("SemanticsEventTopic");
    
    class SemanticsObject;

    class Semantics {
        friend class SemanticsRepository;
        size_t m_hash = 0;
        std::list<SemanticsObject*> m_holders;
        std::list<Semantics*> m_predecessors;
        std::list<Semantics*> m_successors;
    public:
        Semantics(const std::list<Semantics*>& predecessors, size_t hash = 0)
        {
            addToHash(hash);
            for (auto predecessor : predecessors) {
                addPredecessor(predecessor);
                addToHash(predecessor->getHash());
            }
        }

        size_t getHash() const {
            return m_hash;
        }

        bool isSource() const {
            return m_predecessors.empty();
        }

        const std::list<SemanticsObject*>& getHolders() const {
            return m_holders;
        }

        const std::list<Semantics*>& getPredecessors() {
            return m_predecessors;
        }

        const std::list<Semantics*>& getSuccessors() {
            return m_successors;
        }

        bool isEqual(const Semantics* other) const {
            return m_hash == other->m_hash;
        }

        virtual std::string toString() const = 0;

    private:
        void addPredecessor(Semantics* predecessor) {
            m_predecessors.push_back(predecessor);
            predecessor->m_successors.push_back(this);
        }

    protected:
        template<typename T>
        void addToHash(const T& value) {
            boost::hash_combine(m_hash, value);
        }
    };

    class DataTypeSemantics : public Semantics {
        DataType* m_dataType;
    public:
        DataTypeSemantics(
            DataType* dataType,
            size_t hash = 0,
            const std::list<Semantics*>& predecessors = {}
        )
            : Semantics(predecessors, hash)
            , m_dataType(dataType)
        {
            addToHash(dataType->getName());
        }

        DataType* getDataType() const {
            return m_dataType;
        }

        std::string toString() const override {
            return m_dataType->getName();
        }
    };

    class SemanticsObject {
        friend class SemanticsRepository;
        size_t m_hash;
        std::list<Semantics*> m_semantics;
        std::list<std::shared_ptr<ircode::Variable>> m_variables;
    public:
        SemanticsObject(size_t hash)
            : m_hash(hash)
        {}

        const std::list<Semantics*>& getSemantics() const {
            return m_semantics;
        }

        const std::list<std::shared_ptr<ircode::Variable>>& getVariables() const {
            return m_variables;
        }
    };

    class SemanticsRepository
    {
        std::shared_ptr<EventPipe> m_eventPipe;
        std::list<SemanticsObject> m_objects;
        std::map<size_t, std::unique_ptr<Semantics>> m_semantics;
        std::map<ircode::Variable*, SemanticsObject*> m_variableToObject;
        std::map<size_t, SemanticsObject*> m_hashToObject;
    public:
        SemanticsRepository(std::shared_ptr<EventPipe> eventPipe)
            : m_eventPipe(eventPipe)
        {}

        const std::list<SemanticsObject>& getObjects() const {
            return m_objects;
        }

        SemanticsObject* createObject(size_t hash = 0) {
            m_objects.push_back({ hash });
            if (hash != 0) {
                m_hashToObject[hash] = &m_objects.back();
            }
            return &m_objects.back();
        }

        void removeObject(SemanticsObject* object) {
            for (auto variable : object->m_variables) {
                m_variableToObject.erase(variable.get());
            }
            if (object->m_hash != 0) {
                m_hashToObject.erase(object->m_hash);
            }
            m_objects.remove_if([&](const SemanticsObject& obj) {
                return &obj == object;
            });
        }

        SemanticsObject* getObject(std::shared_ptr<ircode::Variable> variable) {
            auto it = m_variableToObject.find(variable.get());
            if (it != m_variableToObject.end()) {
                return it->second;
            }
            return nullptr;
        }

        SemanticsObject* getOrCreateObject(std::shared_ptr<ircode::Variable> variable) {
            auto object = getObject(variable);
            if (object == nullptr) {
                object = createObject();
                bindVariableWithObject(variable, object);
            }
            return object;
        }

        SemanticsObject* getObjectByHash(size_t hash) {
            auto it = m_hashToObject.find(hash);
            if (it != m_hashToObject.end()) {
                return it->second;
            }
            return nullptr;
        }

        void bindVariableWithObject(std::shared_ptr<ircode::Variable> variable, SemanticsObject* object) {
            object->m_variables.push_back(variable);
            m_variableToObject[variable.get()] = object;
        }

        void unbindVariableWithObject(std::shared_ptr<ircode::Variable> variable, SemanticsObject* object) {
            object->m_variables.remove_if([&](const std::shared_ptr<ircode::Variable>& var) {
                return var == variable;
            });
            m_variableToObject.erase(variable.get());
        }

        bool addSemantics(std::unique_ptr<Semantics> sem, SemanticsObject* object) {
            auto it = m_semantics.find(sem->getHash());
            if (it == m_semantics.end()) {
                it = m_semantics.insert({ sem->getHash(), std::move(sem) }).first;
            }
            auto semPtr = it->second.get();
            if (std::find(object->m_semantics.begin(), object->m_semantics.end(), semPtr) != object->m_semantics.end()) {
                return false;
            }
            object->m_semantics.push_back(semPtr);
            semPtr->m_holders.push_back(object);
            return true;
        }

        void removeSemantics(std::list<Semantics*> startSemantics) {
            std::list<Semantics*> allSemanticsToRemove;
            std::list<Semantics*> successors = startSemantics;
            while (!successors.empty()) {
                auto semToRemove = successors.front();
                successors.pop_front();
                allSemanticsToRemove.push_back(semToRemove);
                for (auto successor : semToRemove->m_successors) {
                    successors.push_back(successor);
                }
            }
            for (auto sem : allSemanticsToRemove) {
                for (auto holder : sem->m_holders) {
                    holder->m_semantics.remove_if([&](Semantics* s) {
                        return s == sem;
                    });
                }
                m_semantics.erase(sem->getHash());
            }
        }

        void removeSourceSemantics(std::shared_ptr<ircode::Variable> variable) {
            if (auto object = getObject(variable)) {
                std::list<Semantics*> sourceSemantics;
                for (auto sem : object->m_semantics) {
                    if (sem->isSource()) {
                        sourceSemantics.push_back(sem);
                    }
                }
                removeSemantics(sourceSemantics);
            }
        }
    };

    class SemanticsPropagator
    {
    protected:
        SemanticsRepository* m_repository;
    public:
        SemanticsPropagator(SemanticsRepository* repository)
            : m_repository(repository)
        {}

        virtual void propagate(ResearcherPropagationContext& ctx) = 0;
    };

    class BaseSemanticsPropagator : public SemanticsPropagator
    {
    public:
        BaseSemanticsPropagator(SemanticsRepository* repository)
            : SemanticsPropagator(repository)
        {}

        void propagate(ResearcherPropagationContext& ctx) override {
            auto op = ctx.operation;
            auto output = op->getOutput();
            auto function = op->getBlock()->getFunction();
            if (op->getId() == ircode::OperationId::COPY ||
                op->getId() == ircode::OperationId::REF ||
                op->getId() == ircode::OperationId::LOAD)
            {
                size_t sourceHash = 0;
                if (auto dataType = findDataTypeFromFuncSignature(output, function, sourceHash)) {
                    auto object = m_repository->getOrCreateObject(output);
                    if (m_repository->addSemantics(std::make_unique<DataTypeSemantics>(dataType, sourceHash), object)) {
                        ctx.markValueAsAffected(output);
                    }
                }
            }
            // TODO: copy, ref, load...
        }

    private:
        DataType* findDataTypeFromFuncSignature(std::shared_ptr<sda::ircode::Variable> variable, ircode::Function* function, size_t& hash) {
            boost::hash_combine(hash, "func_signature");
            boost::hash_combine(hash, function->getEntryOffset());
            auto signatureDt = function->getFunctionSymbol()->getSignature();
            auto paramVars = function->getParamVariables();
            for (size_t i = 0; i < paramVars.size(); ++i) {
                if (variable == paramVars[i]) {
                    boost::hash_combine(hash, i);
                    return signatureDt->getParameters()[i]->getDataType();
                }
            }
            if (variable == function->getReturnVariable()) {
                boost::hash_combine(hash, -1);
                return signatureDt->getReturnType();
            }
            return nullptr;
        }
    };

    class SemanticsResearcher
    {
        ircode::Program* m_program;
        SemanticsRepository* m_semanticsRepo;
        DataFlowRepository* m_dataFlowRepo;
        std::list<std::unique_ptr<SemanticsPropagator>> m_propagators;

        class IRcodeEventHandler
        {
            SemanticsResearcher* m_researcher;

            void handleFunctionDecompiled(const ircode::FunctionDecompiledEvent& event) {
                ResearcherPropagationContext ctx;
                for (auto block : event.blocks) { 
                    for (auto& op : block->getOperations()) {
                        ctx.addNextOperation(op.get());
                    }
                }

                auto ctx1 = ctx;
                ctx1.collect([&]() {
                    m_researcher->createSemanticsObject(ctx1);
                });

                auto ctx2 = ctx;
                ctx2.collect([&]() {
                    m_researcher->propagate(ctx2);
                });
            }

            void handleFunctionSignatureChangedEvent(const ircode::FunctionSignatureChangedEvent& event) {
                ResearcherPropagationContext ctx;
                for (auto paramVar : event.m_oldParamVars) {
                    m_researcher->m_semanticsRepo->removeSourceSemantics(paramVar);
                    ctx.addNextOperation(paramVar->getSourceOperation());
                }
                if (event.m_oldReturnVar) {
                    m_researcher->m_semanticsRepo->removeSourceSemantics(event.m_oldReturnVar);
                    ctx.addNextOperation(event.m_oldReturnVar->getSourceOperation());
                }
                for (auto paramVar : event.function->getParamVariables()) {
                    ctx.addNextOperation(paramVar->getSourceOperation());
                }
                if (auto returnVar = event.function->getReturnVariable()) {
                    ctx.addNextOperation(returnVar->getSourceOperation());
                }
                ctx.collect([&]() {
                    m_researcher->propagate(ctx);
                });
            }

            void handleOperationRemoved(const ircode::OperationRemovedEvent& event) {
                auto output = event.op->getOutput();
                if (auto outputObject = m_researcher->m_semanticsRepo->getObject(output)) {
                    m_researcher->m_semanticsRepo->unbindVariableWithObject(output, outputObject);
                    if (outputObject->getVariables().empty()) {
                        m_researcher->m_semanticsRepo->removeObject(outputObject);
                    }
                }
            }
        public:
            IRcodeEventHandler(SemanticsResearcher* researcher) : m_researcher(researcher) {}

            std::shared_ptr<EventPipe> getEventPipe() {
                auto pipe = EventPipe::New();
                pipe->subscribeMethod(this, &IRcodeEventHandler::handleFunctionDecompiled);
                pipe->subscribeMethod(this, &IRcodeEventHandler::handleFunctionSignatureChangedEvent);
                pipe->subscribeMethod(this, &IRcodeEventHandler::handleOperationRemoved);
                return pipe;
            }
        };
        IRcodeEventHandler m_ircodeEventHandler;
        // TODO: context event handlerr (for signature, symbol table changes and etc)
    public:
        SemanticsResearcher(
            ircode::Program* program,
            SemanticsRepository* semanticsRepo,
            DataFlowRepository* dataFlowRepo
        )
            : m_program(program)
            , m_semanticsRepo(semanticsRepo)
            , m_dataFlowRepo(dataFlowRepo)
            , m_ircodeEventHandler(this)
        {}

        std::shared_ptr<EventPipe> getEventPipe() {
            return m_ircodeEventHandler.getEventPipe();
        }

        void addPropagator(std::unique_ptr<SemanticsPropagator> propagator) {
            m_propagators.push_back(std::move(propagator));
        }

        void propagate(ResearcherPropagationContext& ctx) {
            for (auto& propagator : m_propagators) {
                propagator->propagate(ctx);
            }
        }

    private:
        void createSemanticsObject(ResearcherPropagationContext& ctx) {
            auto output = ctx.operation->getOutput();
            if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(ctx.operation)) {
                auto input = unaryOp->getInput();
                if (unaryOp->getId() == ircode::OperationId::COPY) {
                    if (auto inputVar = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                        if (auto inputObject = m_semanticsRepo->getOrCreateObject(inputVar)) {
                            if (auto outputObject = m_semanticsRepo->getObject(output)) {
                                m_semanticsRepo->removeObject(outputObject);
                            }
                            m_semanticsRepo->bindVariableWithObject(output, inputObject);
                        }
                    }
                }
                // INT_ADD, ...
            }
            m_semanticsRepo->getOrCreateObject(output);
        }
    };
};