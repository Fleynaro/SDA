#pragma once
#include "ResearcherHelper.h"
#include "DataFlowResearcher.h"
#include "ClassResearcher.h"
#include "SDA/Core/DataType/ScalarDataType.h"
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
        size_t m_sourceHash;
        std::list<SemanticsObject*> m_holders;
        std::list<Semantics*> m_predecessors;
        std::list<Semantics*> m_successors;
    public:
        Semantics(const std::list<Semantics*>& predecessors, size_t sourceHash = 0)
            : m_sourceHash(sourceHash)
        {
            addToHash(sourceHash);
            for (auto predecessor : predecessors) {
                addPredecessor(predecessor);
                addToHash(predecessor->getHash());
            }
        }

        size_t getHash() const {
            return m_hash;
        }

        size_t getSourceHash() const {
            return m_sourceHash;
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
            size_t sourceHash = 0,
            const std::list<Semantics*>& predecessors = {}
        )
            : Semantics(predecessors, sourceHash)
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

    size_t FunctionParamToHash(ircode::Function* function, size_t index) {
        size_t hash = 0;
        boost::hash_combine(hash, "func_signature");
        boost::hash_combine(hash, function->getEntryOffset());
        boost::hash_combine(hash, index);
        return hash;
    }

    size_t FunctionReturnToHash(ircode::Function* function) {
        return FunctionParamToHash(function, -1);
    }

    class SemanticsObject {
        friend class SemanticsRepository;
        std::list<Semantics*> m_semantics;
        std::list<std::shared_ptr<ircode::Variable>> m_variables;
    public:
        SemanticsObject() {}

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
        std::map<ircode::Variable*, SemanticsObject*> m_variableToObject;
        std::map<size_t, std::unique_ptr<Semantics>> m_semantics;
        std::map<size_t, std::list<Semantics*>> m_hashToSourceSemantics;
    public:
        SemanticsRepository(std::shared_ptr<EventPipe> eventPipe)
            : m_eventPipe(eventPipe)
        {}

        const std::list<SemanticsObject>& getObjects() const {
            return m_objects;
        }

        SemanticsObject* createObject() {
            m_objects.emplace_back();
            return &m_objects.back();
        }

        void removeObject(SemanticsObject* object) {
            removeSemantics(object->m_semantics);
            for (auto variable : object->m_variables) {
                m_variableToObject.erase(variable.get());
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
                auto sem = it->second.get();
                if (sem->isSource()) {
                    m_hashToSourceSemantics[sem->getSourceHash()].push_back(sem);
                }
            }
            return addSemanticsToObject(it->second.get(), object);
        }

        bool addSemanticsToObject(Semantics* sem, SemanticsObject* object) {
            if (std::find(object->m_semantics.begin(), object->m_semantics.end(), sem) != object->m_semantics.end()) {
                return false;
            }
            object->m_semantics.push_back(sem);
            sem->m_holders.push_back(object);
            return true;
        }

        void removeSemantics(Semantics* sem) {
            for (auto holder : sem->m_holders) {
                holder->m_semantics.remove_if([&](Semantics* s) {
                    return s == sem;
                });
            }
            m_semantics.erase(sem->getHash());
            if (sem->isSource()) {
                auto it = m_hashToSourceSemantics.find(sem->getSourceHash());
                if (it != m_hashToSourceSemantics.end()) {
                    it->second.remove_if([&](Semantics* s) {
                        return s == sem;
                    });
                    if (it->second.empty()) {
                        m_hashToSourceSemantics.erase(it);
                    }
                }
            }
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
                removeSemantics(sem);
            }
        }

        void removeSemantics(size_t sourceHash) {
            auto it = m_hashToSourceSemantics.find(sourceHash);
            if (it != m_hashToSourceSemantics.end()) {
                removeSemantics(it->second);
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
        ircode::Program* m_program;
    public:
        BaseSemanticsPropagator(ircode::Program* program, SemanticsRepository* repository)
            : SemanticsPropagator(repository)
            , m_program(program)
        {}

        void propagate(ResearcherPropagationContext& ctx) override {
            auto op = ctx.operation;
            auto opId = op->getId();
            auto output = op->getOutput();
            auto outputSize = output->getSize();
            auto function = op->getBlock()->getFunction();
            if (opId == ircode::OperationId::COPY ||
                opId == ircode::OperationId::REF ||
                opId == ircode::OperationId::LOAD)
            {
                if (auto object = m_repository->getObject(output)) {
                    size_t sourceHash = 0;
                    if (auto dataType = findDataTypeFromFuncSignature(output, function, sourceHash)) {
                        if (m_repository->addSemantics(std::make_unique<DataTypeSemantics>(dataType, sourceHash), object)) {
                            markObjectAsAffected(ctx, object);
                        }
                    }
                }
            }

            if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(op)) {
                auto input = unaryOp->getInput();

                if (opId == ircode::OperationId::INT_2COMP) {
                    auto dataType = getScalarDataType(ScalarType::SignedInt, outputSize);
                    setDataTypeFor(ctx, output, dataType);
                } else if (opId == ircode::OperationId::BOOL_NEGATE) {
                    auto dataType = findDataType("bool");
                    setDataTypeFor(ctx, input, dataType);
                    setDataTypeFor(ctx, output, dataType);
                }
                else if (
                    opId == ircode::OperationId::FLOAT_NEG ||
                    opId == ircode::OperationId::FLOAT_ABS ||
                    opId == ircode::OperationId::FLOAT_SQRT
                ) {
                    auto dataType = getScalarDataType(ScalarType::FloatingPoint, outputSize);
                    setDataTypeFor(ctx, input, dataType);
                    setDataTypeFor(ctx, output, dataType);
                }
            } else if (auto binaryOp = dynamic_cast<const ircode::BinaryOperation*>(op)) {
                auto input1 = binaryOp->getInput1();
                auto input2 = binaryOp->getInput2();

                if (
                    opId == ircode::OperationId::INT_ADD ||
                    opId == ircode::OperationId::INT_SUB ||
                    opId == ircode::OperationId::INT_MULT ||
                    opId == ircode::OperationId::INT_DIV ||
                    opId == ircode::OperationId::INT_REM
                ) {
                    if (auto object = m_repository->getObject(output)) {
                        for (auto& input : {input1, input2}) {
                            if (auto inputVar = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                                if (auto inputObject = m_repository->getObject(inputVar)) {
                                    for (auto inputSem : inputObject->getSemantics()) {
                                        if (auto dtInputSem = dynamic_cast<DataTypeSemantics*>(inputSem)) {
                                            if (dtInputSem->getDataType()->isScalar(ScalarType::SignedInt)) {
                                                if (m_repository->addSemanticsToObject(dtInputSem, object)) {
                                                    markObjectAsAffected(ctx, object);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else if (
                    opId == ircode::OperationId::INT_SDIV ||
                    opId == ircode::OperationId::INT_SREM
                ) {
                    auto dataType = getScalarDataType(ScalarType::SignedInt, outputSize);
                    setDataTypeFor(ctx, output, dataType);
                } else if (
                    opId >= ircode::OperationId::INT_EQUAL &&
                    opId <= ircode::OperationId::INT_LESSEQUAL ||
                    opId >= ircode::OperationId::FLOAT_EQUAL &&
                    opId <= ircode::OperationId::FLOAT_LESSEQUAL
                ) {
                    auto booleanDt = findDataType("bool");
                    setDataTypeFor(ctx, output, booleanDt);
                    if (binaryOp->getId() >= ircode::OperationId::FLOAT_EQUAL &&
                        binaryOp->getId() <= ircode::OperationId::FLOAT_LESSEQUAL)
                        {
                            auto floatScalarDt = getScalarDataType(ScalarType::FloatingPoint, outputSize);
                            setDataTypeFor(ctx, input1, floatScalarDt);
                            setDataTypeFor(ctx, input2, floatScalarDt);
                }
                } else if (
                    opId >= ircode::OperationId::BOOL_NEGATE &&
                    opId <= ircode::OperationId::BOOL_OR
                ) {
                    auto dataType = findDataType("bool");
                    setDataTypeFor(ctx, input1, dataType);
                    setDataTypeFor(ctx, input2, dataType);
                    setDataTypeFor(ctx, output, dataType);
                } else if (
                    opId >= ircode::OperationId::FLOAT_ADD &&
                    opId <= ircode::OperationId::FLOAT_SQRT
                ) {
                    auto dataType = getScalarDataType(ScalarType::FloatingPoint, outputSize);
                    setDataTypeFor(ctx, input1, dataType);
                    setDataTypeFor(ctx, input2, dataType);
                    setDataTypeFor(ctx, output, dataType);
                }
            }
        }

    private:
        DataType* findDataTypeFromFuncSignature(std::shared_ptr<sda::ircode::Variable> variable, ircode::Function* function, size_t& hash) {
            auto signatureDt = function->getFunctionSymbol()->getSignature();
            auto paramVars = function->getParamVariables();
            for (size_t i = 0; i < paramVars.size(); ++i) {
                if (variable == paramVars[i]) {
                    hash = FunctionParamToHash(function, i);
                    return signatureDt->getParameters()[i]->getDataType();
                }
            }
            if (variable == function->getReturnVariable()) {
                hash = FunctionReturnToHash(function);
                return signatureDt->getReturnType();
            }
            return nullptr;
        }

        ScalarDataType* getScalarDataType(ScalarType scalarType, size_t size) const {
            return getContext()->getDataTypes()->getScalar(scalarType, size);
        }

        DataType* findDataType(const std::string& name) const {
            auto dataType = getContext()->getDataTypes()->getByName(name);
            assert(dataType);
            return dataType;
        }

        void setDataTypeFor(ResearcherPropagationContext& ctx, std::shared_ptr<ircode::Value> value, DataType* dataType) {
            if (auto var = std::dynamic_pointer_cast<ircode::Variable>(value)) {
                if (auto object = m_repository->getObject(var)) {
                    size_t sourceHash = 0;
                    boost::hash_combine(sourceHash, ctx.operation);
                    if (m_repository->addSemantics(std::make_unique<DataTypeSemantics>(dataType, sourceHash), object)) {
                        markObjectAsAffected(ctx, object);
                    }
                }
            }
        }

        void markObjectAsAffected(ResearcherPropagationContext& ctx, SemanticsObject* object) {
            for (auto& var : object->getVariables()) {
                ctx.markValueAsAffected(var);
            }
        }

        Context* getContext() const {
            return m_program->getGlobalSymbolTable()->getContext();
        }
    };

    class SemanticsResearcher
    {
        ircode::Program* m_program;
        SemanticsRepository* m_semanticsRepo;
        DataFlowRepository* m_dataFlowRepo;
        ClassRepository* m_classRepo;
        std::list<std::unique_ptr<SemanticsPropagator>> m_propagators;

        class EventHandler
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
                for (size_t i = 0; i < event.m_oldParamVars.size(); ++i) {
                    auto paramVar = event.m_oldParamVars[i];
                    m_researcher->m_semanticsRepo->removeSemantics(FunctionParamToHash(event.function, i));
                    ctx.addNextOperation(paramVar->getSourceOperation());
                }
                if (event.m_oldReturnVar) {
                    m_researcher->m_semanticsRepo->removeSemantics(FunctionReturnToHash(event.function));
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

            void handleStructureUpdatedEventBatch(const EventBatch<StructureUpdatedEvent>& event) {
                std::list<Structure*> updatedStructures;
                for (auto& e : event.events) {
                    updatedStructures.push_back(e.structure);
                }
                std::set<SemanticsObject*> objectsToRemove;
                // found groups of identical variables constitute semantic objects
                std::list<std::list<std::shared_ptr<ircode::Variable>>> groupsOfIdenticVariables;
                while (!updatedStructures.empty()) {
                    auto structure = updatedStructures.front();
                    // get all structures in the group
                    std::set<Structure*> structuresInGroup;
                    m_researcher->m_classRepo->gatherStructuresInGroup(structure, structuresInGroup);
                    // find objects to remove
                    for (auto structure : structuresInGroup) {
                        if (structure->sourceNode) {
                            if (auto sourceVar = structure->sourceNode->getVariable()) {
                                if (auto object = m_researcher->m_semanticsRepo->getObject(sourceVar)) {
                                    objectsToRemove.insert(object);
                                }
                            }
                        }
                    }
                    // find new variables for each label
                    auto& group = groupsOfIdenticVariables.emplace_back();
                    for (auto structure : structuresInGroup) {
                        for (auto node : structure->linkedNodes) {
                            if (auto var = node->getVariable()) {
                                // TODO: define label
                                group.push_back(var);
                            }
                        }
                    }
                    // remove from updated structures
                    for (auto structure : structuresInGroup) {
                        auto it = std::find(updatedStructures.begin(), updatedStructures.end(), structure);
                        if (it != updatedStructures.end()) {
                            updatedStructures.erase(it);
                        }
                    }
                }

                ResearcherPropagationContext ctx;
                // remove all the related semantic objects
                for (auto object : objectsToRemove) {
                    for (auto var : object->getVariables()) {
                        ctx.addNextOperation(var->getSourceOperation());
                    }
                    m_researcher->m_semanticsRepo->removeObject(object);
                }
                // create new semantic objects based on the groups
                for (auto group : groupsOfIdenticVariables) {
                    auto newObject = m_researcher->m_semanticsRepo->createObject();
                    for (auto var : group) {
                        m_researcher->m_semanticsRepo->bindVariableWithObject(var, newObject);
                        ctx.addNextOperation(var->getSourceOperation());
                    }
                }

                ctx.collect([&]() {
                    m_researcher->propagate(ctx);
                });
            }

            void handleStructureRemovedEvent(const StructureRemovedEvent& event) {
                // find related semantic object (always single), remove structure's variables from it and clean it from all semantic
                ResearcherPropagationContext ctx;
                ResearcherPropagationContext ctx2;
                if (event.structure->sourceNode) {
                    if (auto sourceVar = event.structure->sourceNode->getVariable()) {
                        if (auto object = m_researcher->m_semanticsRepo->getObject(sourceVar)) {
                            for (auto var : object->getVariables()) {
                                ctx.addNextOperation(var->getSourceOperation());
                            }
                            for (auto node : event.structure->linkedNodes) {
                                if (auto var = node->getVariable()) {
                                    // remove structure's variables
                                    m_researcher->m_semanticsRepo->unbindVariableWithObject(var, object);
                                    ctx2.addNextOperation(var->getSourceOperation());
                                }
                            }
                            // clean semantic object
                            m_researcher->m_semanticsRepo->removeSemantics(object->getSemantics());
                        }
                    }
                }
                ctx2.collect([&]() {
                    // variables which are now free of structure are to be reassigned to new semantic objects
                    m_researcher->createSemanticsObject(ctx2);
                });
                ctx.collect([&]() {
                    m_researcher->propagate(ctx);
                });
            }
        public:
            EventHandler(SemanticsResearcher* researcher) : m_researcher(researcher) {}

            std::shared_ptr<EventPipe> getEventPipe() {
                auto pipe = EventPipe::New();
                pipe
                    ->connect(StructureRepository::GetOptimizedEventPipe())
                    ->subscribeMethod(this, &EventHandler::handleStructureUpdatedEventBatch);
                pipe->subscribeMethod(this, &EventHandler::handleStructureRemovedEvent);
                pipe->subscribeMethod(this, &EventHandler::handleFunctionDecompiled);
                pipe->subscribeMethod(this, &EventHandler::handleFunctionSignatureChangedEvent);
                pipe->subscribeMethod(this, &EventHandler::handleOperationRemoved);
                return pipe;
            }
        };
        EventHandler m_eventHandler;
    public:
        SemanticsResearcher(
            ircode::Program* program,
            SemanticsRepository* semanticsRepo,
            DataFlowRepository* dataFlowRepo,
            ClassRepository* classRepo
        )
            : m_program(program)
            , m_semanticsRepo(semanticsRepo)
            , m_dataFlowRepo(dataFlowRepo)
            , m_classRepo(classRepo)
            , m_eventHandler(this)
        {}

        std::shared_ptr<EventPipe> getEventPipe() {
            return m_eventHandler.getEventPipe();
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