#pragma once
#include "ResearcherHelper.h"
#include "DataFlowResearcher.h"
#include "ClassResearcher.h"
#include "SDA/Core/DataType/ScalarDataType.h"
#include "SDA/Core/DataType/PointerDataType.h"
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
        std::list<Semantics*> m_successors;
    public:
        Semantics() = default;

        size_t getHash() const {
            return m_hash;
        }

        const std::list<Semantics*>& getSuccessors() const {
            return m_successors;
        }

        virtual std::list<Semantics*> getPredecessors() const {
            return {};
        }

        bool isEqual(const Semantics* other) const {
            return m_hash == other->m_hash;
        }

        virtual std::string toString() const = 0;

    protected:
        template<typename T>
        void addToHash(const T& value) {
            boost::hash_combine(m_hash, value);
        }
    };

    class DataTypeSemantics : public Semantics {
        DataType* m_dataType;
    public:
        DataTypeSemantics(DataType* dataType)
            : m_dataType(dataType)
        {
            addToHash("data_type");
            addToHash(dataType->getName());
        }

        DataType* getDataType() const {
            return m_dataType;
        }

        std::string toString() const override {
            return m_dataType->getName();
        }
    };

    class SymbolPointerSemantics : public Semantics {
        SymbolTable* m_symbolTable;
        Offset m_offset;
    public:
        SymbolPointerSemantics(SymbolTable* symbolTable, Offset offset)
            : m_symbolTable(symbolTable)
            , m_offset(offset)
        {
            addToHash("symbol_pointer");
            addToHash(symbolTable);
            addToHash(offset);
        }

        SymbolTable* getSymbolTable() const {
            return m_symbolTable;
        }

        Offset getOffset() const {
            return m_offset;
        }

        std::string toString() const override {
            return "symbol_pointer(0x" + utils::ToHex(m_offset) + ")";
        }
    };

    class SymbolLoadSemantics : public Semantics {
        SymbolPointerSemantics* m_pointerSem;
        size_t m_loadSize;
    public:
        SymbolLoadSemantics(SymbolPointerSemantics* pointerSem, size_t loadSize)
            : m_pointerSem(pointerSem)
            , m_loadSize(loadSize)
        {
            addToHash("symbol_load");
            addToHash(pointerSem);
            addToHash(loadSize);
        }

        SymbolPointerSemantics* getPointerSemantics() {
            return m_pointerSem;
        }

        size_t getLoadSize() const {
            return m_loadSize;
        }

        std::list<Semantics*> getPredecessors() const override {
            return { m_pointerSem };
        }

        std::string toString() const override {
            return "symbol_load(0x" + utils::ToHex(m_pointerSem->getOffset()) + ":" + std::to_string(m_loadSize) + ")";
        }
    };

    class FunctionParameterSemantics : public Semantics {
        ircode::Function* m_function;
        size_t m_index;
    public:
        FunctionParameterSemantics(
            ircode::Function* function,
            size_t index
        )
            : m_function(function)
            , m_index(index)
        {
            addToHash("func_param");
            addToHash(function->getEntryOffset());
            addToHash(index);
        }

        std::string toString() const override {
            return std::string("param") + std::to_string(m_index + 1);
        }
    };

    class FunctionReturnSemantics : public Semantics {
        ircode::Function* m_function;
    public:
        FunctionReturnSemantics(ircode::Function* function)
            : m_function(function)
        {
            addToHash("func_return");
            addToHash(function->getEntryOffset());
        }

        std::string toString() const override {
            return "return";
        }
    };

    class OperationSemantics : public Semantics {
        const ircode::Operation* m_operation;
    public:
        OperationSemantics(const ircode::Operation* operation)
            : m_operation(operation)
        {
            addToHash("operation");
            addToHash(operation);
        }

        std::string toString() const override {
            return "operation";
        }
    };

    class ClassLabelFieldSemantics : public Semantics {
        ClassLabelInfo m_info;
    public:
        ClassLabelFieldSemantics(const ClassLabelInfo& info)
            : m_info(info)
        {
            addToHash("class_label_field");
            addToHash(info.structureInstrOffset.fullOffset);
            addToHash(info.sourceInstrOffset.fullOffset);
            addToHash(info.labelOffset);
        }

        const ClassLabelInfo& getClassLabelInfo() const {
            return m_info;
        }

        std::string toString() const override {
            return "class_label_field(0x" + utils::ToHex(m_info.labelOffset) + ")";
        }
    };

    class HolderSemantics : public Semantics {
        std::list<Semantics*> m_predecessors;
        Semantics* m_semantics;
        SemanticsObject* m_holder = nullptr;
    public:
        HolderSemantics(Semantics* semantics, const std::list<Semantics*>& predecessors, SemanticsObject* holder)
            : m_semantics(semantics)
            , m_predecessors(predecessors)
            , m_holder(holder)
        {
            m_predecessors.push_back(semantics);
            addToHash("holder");
            for (auto pred : m_predecessors) {
                addToHash(pred);
            }
            addToHash(holder);
        }

        Semantics* getSemantics() const {
            return m_semantics;
        }

        std::list<Semantics*> getPredecessors() const override {
            return m_predecessors;
        }

        SemanticsObject* getHolder() const {
            return m_holder;
        }

        std::string toString() const override {
            return "holder";
        }
    };

    std::list<SemanticsObject*> GetSemanticsHolders(Semantics* sem) {
        if (auto holderSem = dynamic_cast<HolderSemantics*>(sem)) {
            return { holderSem->getHolder() };
        }
        // gather first holders of all successors
        std::list<SemanticsObject*> objects;
        for (auto succ : sem->getSuccessors()) {
            auto holders = GetSemanticsHolders(succ);
            objects.insert(objects.end(), holders.begin(), holders.end());
        }
        return objects;
    }

    class SemanticsObject {
        friend class SemanticsRepository;
        std::list<HolderSemantics*> m_semantics;
        std::list<std::shared_ptr<ircode::Variable>> m_variables;
    public:
        SemanticsObject() {}

        const std::list<HolderSemantics*>& getSemantics() const {
            return m_semantics;
        }

        const std::list<std::shared_ptr<ircode::Variable>>& getVariables() const {
            return m_variables;
        }
    };

    // used where object get semantics or all object semantics removed (object cleaned)
    void MarkObjectAsAffected(ResearcherPropagationContext& ctx, SemanticsObject* object, DataFlowRepository* dataFlowRepo) {
        for (auto& var : object->getVariables()) {
            ctx.markValueAsAffected(var);
            // see test SemanticsResearcherTest.MutualFunction
            if (auto dataFlowNode = dataFlowRepo->getNode(var)) {
                for (auto succ : dataFlowNode->successors) {
                    if (succ->type == DataFlowNode::Copy) {
                        if (auto succVar = succ->getVariable()) {
                            ctx.markValueAsAffected(succVar);
                        }
                    }
                }
            }
        }
    }

    // used where specific source semantics of object removed by hash
    void AddObjectVariablesToContext(ResearcherPropagationContext& ctx, SemanticsObject* object) {
        for (auto& var : object->getVariables()) {
            ctx.addNextOperation(var->getSourceOperation());
        }
    }

    class SemanticsRepository
    {
        std::shared_ptr<EventPipe> m_eventPipe;
        std::list<SemanticsObject> m_objects;
        std::map<ircode::Variable*, SemanticsObject*> m_variableToObject;
        std::map<size_t, std::unique_ptr<Semantics>> m_semantics;
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

        // remove all semantics of object
        void cleanObject(SemanticsObject* object) {
            std::list<Semantics*> semanticsToRemove;
            for (auto sem : object->m_semantics) {
                semanticsToRemove.push_back(sem);
            }
            removeSemanticsChain(semanticsToRemove);
        }

        void removeObject(SemanticsObject* object) {
            cleanObject(object);
            // remove variables from map
            for (auto variable : object->m_variables) {
                m_variableToObject.erase(variable.get());
            }
            // remove object itself
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

        template<typename T>
        T* addSemantics(const T& sem, bool alwaysReturn = true) {
            auto semPtr = getSemantics(sem.getHash());
            if (semPtr) {
                if (alwaysReturn) {
                    return static_cast<T*>(semPtr);
                }
                return nullptr;
            }
            return static_cast<T*>(addNewSemantics(std::make_unique<T>(sem)));
        }

        bool addSemanticsToObject(Semantics* sem, const std::list<Semantics*>& predecessors, SemanticsObject* object) {
            if (auto holderSem = addSemantics(HolderSemantics(sem, predecessors, object), false)) {
                if (object) {
                    object->m_semantics.push_back(holderSem);
                }
                return true;
            }
            return false;
        }

        class Adder {
            SemanticsRepository* m_repository;
            std::list<Semantics*> m_semantics;
            Semantics* m_current = nullptr;
            Semantics* m_result = nullptr;
            std::list<Semantics*> m_predecessors;
        public:
            Adder(SemanticsRepository* repository)
                : m_repository(repository)
            {}

            // Add already existing semantics
            Adder& addPointer(Semantics* sem) {
                assert(m_current == nullptr);
                m_current = sem;
                return *this;
            }

            // Add new semantics
            template<typename T>
            Adder& add(const T& sem) {
                return addPointer(m_repository->addSemantics(sem));
            }

            // Add already existing dependency semantics
            Adder& addDependencyPointer(Semantics* sem, SemanticsObject* object = nullptr) {
                m_predecessors.push_back(sem);
                if (object) {
                    m_repository->addSemanticsToObject(sem, {}, object);
                }
                return *this;
            }

            // Add new dependency semantics
            template<typename T>
            Adder& addDependency(const T& sem, SemanticsObject* object = nullptr) {
                return addDependencyPointer(m_repository->addSemantics(sem), object);
            }

            bool addTo(SemanticsObject* object) {
                return m_repository->addSemanticsToObject(m_current, m_predecessors, object);
            }
        };

        Adder getAdder() {
            return Adder(this);
        }

        void removeSemanticsChain(std::list<Semantics*> startSemantics) {
            // gather all semantics to remove
            std::set<Semantics*> allSemanticsToRemove;
            std::list<Semantics*> successors = startSemantics;
            while (!successors.empty()) {
                auto semToRemove = successors.front();
                successors.pop_front();
                allSemanticsToRemove.insert(semToRemove);
                for (auto successor : semToRemove->m_successors) {
                    if (allSemanticsToRemove.find(successor) != allSemanticsToRemove.end())
                        continue;
                    successors.push_back(successor);
                }
            }

            // disconnect all semantics from their predecessors including all predecessors without successors
            auto allSemanticsToDisconnect = allSemanticsToRemove;
            while (!allSemanticsToDisconnect.empty()) {
                auto semToDisconnect = *allSemanticsToDisconnect.begin();
                allSemanticsToDisconnect.erase(allSemanticsToDisconnect.begin());
                disconnectSemantics(semToDisconnect);
                for (auto predecessor : semToDisconnect->getPredecessors()) {
                    if (allSemanticsToRemove.find(predecessor) != allSemanticsToRemove.end())
                        continue;
                    // if semantics without successors (by itself), add it to remove list
                    if (predecessor->m_successors.empty()) {
                        if (!dynamic_cast<HolderSemantics*>(predecessor)) {
                            allSemanticsToDisconnect.insert(predecessor);
                            allSemanticsToRemove.insert(predecessor);
                        }
                    }
                }
            }

            // remove them
            for (auto sem : allSemanticsToRemove) {
                removeSemantics(sem);
            }
        }

        void removeSemanticsChain(size_t hash) {
            if (auto semantics = getSemantics(hash)) {
                removeSemanticsChain({ semantics });
            }
        }

        Semantics* getSemantics(size_t hash) {
            auto it = m_semantics.find(hash);
            if (it != m_semantics.end()) {
                return it->second.get();
            }
            return nullptr;
        }

    private:
        Semantics* addNewSemantics(std::unique_ptr<Semantics> sem, SemanticsObject* object = nullptr) {
            auto semPtr = sem.get();
            m_semantics[sem->getHash()] = std::move(sem);
            for (auto predecessor : semPtr->getPredecessors()) {
                predecessor->m_successors.push_back(semPtr);
            }
            return semPtr;
        }

        void disconnectSemantics(Semantics* sem) {
            for (auto pred : sem->getPredecessors()) {
                pred->m_successors.remove_if([&](Semantics* s) {
                    return s == sem;
                });
            }
            if (auto holderSem = dynamic_cast<HolderSemantics*>(sem)) {
                holderSem->getHolder()->m_semantics.remove_if([&](HolderSemantics* s) {
                    return s == holderSem;
                });
            }
        }

        void removeSemantics(Semantics* sem) {
            m_semantics.erase(sem->getHash());
        }
    };

    class SemanticsPropagator
    {
    protected:
        SemanticsRepository* m_repository;
        DataFlowRepository* m_dataFlowRepo;

        void markValueAsAffected(ResearcherPropagationContext& ctx, SemanticsObject* object) {
            MarkObjectAsAffected(ctx, object, m_dataFlowRepo);
        }
    public:
        SemanticsPropagator(SemanticsRepository* repository, DataFlowRepository* dataFlowRepo)
            : m_repository(repository)
            , m_dataFlowRepo(dataFlowRepo)
        {}

        virtual void propagate(ResearcherPropagationContext& ctx) = 0;
    };

    class BaseSemanticsPropagator : public SemanticsPropagator
    {
        ircode::Program* m_program;
    public:
        BaseSemanticsPropagator(ircode::Program* program, SemanticsRepository* repository, DataFlowRepository* dataFlowRepo)
            : SemanticsPropagator(repository, dataFlowRepo)
            , m_program(program)
        {}

        void propagate(ResearcherPropagationContext& ctx) override {
            auto op = ctx.operation;
            auto opId = op->getId();
            auto output = op->getOutput();
            auto object = m_repository->getObject(output);
            if (!object) return;
            auto outputSize = output->getSize();
            auto function = op->getBlock()->getFunction();
            auto signatureDt = function->getFunctionSymbol()->getSignature();

            if (opId == ircode::OperationId::COPY ||
                opId == ircode::OperationId::LOAD)
            {
                auto paramVars = function->getParamVariables();
                for (size_t i = 0; i < paramVars.size(); ++i) {
                    if (output == paramVars[i]) {
                        auto dataType = signatureDt->getParameters()[i]->getDataType();
                        if (m_repository->getAdder()
                                .add(DataTypeSemantics(dataType))
                                .addDependency(FunctionParameterSemantics(function, i), object)
                                .addTo(object)
                        ) {
                            markValueAsAffected(ctx, object);
                        }
                    }
                }

                // get semantics from other functions through parameters (presented as data flow nodes)
                // (see test SemanticsResearcherTest.MutualFunction)
                if (auto dataFlowNode = m_dataFlowRepo->getNode(output)) {
                    std::set<SemanticsObject*> predObjects;
                    if (dataFlowNode->type == DataFlowNode::Copy) {
                        for (auto pred : dataFlowNode->predecessors) {
                            if (auto predVar = pred->getVariable()) {
                                if (auto predObject = m_repository->getObject(predVar)) {
                                    if (predObject != object) {
                                        predObjects.insert(predObject);
                                    }
                                }
                            }
                        }
                    }
                    for (auto predObject : predObjects) {
                        for (auto holderPredSem : predObject->getSemantics()) {
                            if (m_repository->getAdder()
                                    .addPointer(holderPredSem->getSemantics())
                                    .addDependencyPointer(holderPredSem)
                                    .addTo(object)
                            ) {
                                markValueAsAffected(ctx, object);
                            }
                        }
                    }
                }
            }

            if (output == function->getReturnVariable()) {
                auto dataType = signatureDt->getReturnType();
                if (m_repository->getAdder()
                        .add(DataTypeSemantics(dataType))
                        .addDependency(FunctionReturnSemantics(function), object)
                        .addTo(object)
                ) {
                    markValueAsAffected(ctx, object);
                }
            }

            if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(op)) {
                auto input = unaryOp->getInput();
                
                if (opId == ircode::OperationId::COPY) {
                    auto outputAddrVal = output->getMemAddress().value;
                    if (auto outputAddrVar = std::dynamic_pointer_cast<ircode::Variable>(outputAddrVal)) {
                        if (auto addrObject = m_repository->getObject(outputAddrVar)) {
                            handleAddress(ctx, addrObject);
                        }
                        size_t constValue;
                        if (ircode::ExtractConstant(input, constValue)) {
                            handleClassLabelFieldOperation(ctx, outputAddrVar);
                        }
                    }
                }
                else if (opId == ircode::OperationId::LOAD) {
                    if (auto inputReg = std::dynamic_pointer_cast<ircode::Register>(input)) {
                        auto symbolTables = getAllSymbolTables(ctx, output);
                        handleSymbolTables(ctx, symbolTables, 0);
                    } else if (auto inputVar = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                        if (auto inputObject = m_repository->getObject(inputVar)) {
                            handleAddress(ctx, inputObject);
                        }
                    }
                }
                else if (opId == ircode::OperationId::INT_2COMP) {
                    auto dataType = getScalarDataType(ScalarType::SignedInt, outputSize);
                    setDataTypeFor(ctx, output, dataType);
                }
                else if (opId == ircode::OperationId::BOOL_NEGATE) {
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
                    for (auto& input : {input1, input2}) {
                        if (auto inputVar = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                            if (auto inputObject = m_repository->getObject(inputVar)) {
                                for (auto holderInputSem : inputObject->getSemantics()) {
                                    if (auto dtInputSem = dynamic_cast<DataTypeSemantics*>(holderInputSem->getSemantics())) {
                                        if (dtInputSem->getDataType()->isScalar(ScalarType::SignedInt)) {
                                            if (m_repository->getAdder()
                                                    .addPointer(dtInputSem)
                                                    .addDependencyPointer(holderInputSem)
                                                    .addTo(object)
                                            ) {
                                                markValueAsAffected(ctx, object);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (opId == ircode::OperationId::INT_ADD ||
                        opId == ircode::OperationId::INT_MULT)
                    {
                        auto linearExpr = ircode::GetLinearExpr(output, true);
                        auto offset = linearExpr.getConstTermValue();
                        auto baseTerms = ircode::ToBaseTerms(linearExpr, getContext()->getPlatform());
                        for (auto& term : baseTerms) {
                            auto symbolTables = getAllSymbolTables(ctx, term);
                            handleSymbolTables(ctx, symbolTables, offset);
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
                    if (opId >= ircode::OperationId::FLOAT_EQUAL &&
                        opId <= ircode::OperationId::FLOAT_LESSEQUAL)
                        {
                            auto floatScalarDt = getScalarDataType(ScalarType::FloatingPoint, outputSize);
                            setDataTypeFor(ctx, input1, floatScalarDt);
                            setDataTypeFor(ctx, input2, floatScalarDt);
                        }
                    if (opId == ircode::OperationId::INT_EQUAL || opId == ircode::OperationId::INT_NOTEQUAL) {
                        if (auto addrValue = ircode::ExtractAddressValue(input1)) {
                            size_t constValue;
                            if (ircode::ExtractConstant(input2, constValue)) {
                                handleClassLabelFieldOperation(ctx, addrValue);
                            }
                        }
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
                } else if (opId == ircode::OperationId::PHI) {
                    for (auto& input : {input1, input2}) {
                        if (auto inputVar = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                            if (auto inputObject = m_repository->getObject(inputVar)) {
                                for (auto holderInputSem : inputObject->getSemantics()) {
                                    if (m_repository->getAdder()
                                            .addPointer(holderInputSem->getSemantics())
                                            .addDependencyPointer(holderInputSem)
                                            .addTo(object)
                                    ) {
                                        markValueAsAffected(ctx, object);
                                    }
                                }
                            }
                        }
                    }
                }
            } else if (auto callOp = dynamic_cast<const ircode::CallOperation*>(ctx.operation)) {
                auto calledFunctions = m_program->getFunctionsByCallInstruction(callOp->getPcodeInstruction());
                if (!calledFunctions.empty()) {
                    auto calledFunction = calledFunctions.front();
                    auto& argValues = callOp->getArguments();
                    auto calledSignatureDt = calledFunction->getFunctionSymbol()->getSignature();
                    auto& params = calledSignatureDt->getParameters();
                    assert(argValues.size() == params.size());
                    for (size_t i = 0; i < argValues.size(); ++i) {
                        auto argValue = argValues[i];
                        if (auto argVar = std::dynamic_pointer_cast<ircode::Variable>(argValue)) {
                            if (auto argObject = m_repository->getObject(argVar)) {
                                auto dataType = params[i]->getDataType();
                                if (m_repository->getAdder()
                                        .add(DataTypeSemantics(dataType))
                                        .addDependency(FunctionParameterSemantics(calledFunction, i), argObject)
                                        .addTo(argObject)
                                ) {
                                    markValueAsAffected(ctx, object);
                                }
                            }
                        }
                    }
                    auto retDataType = signatureDt->getReturnType();
                    if (!retDataType->isVoid()) {
                        if (m_repository->getAdder()
                                .add(DataTypeSemantics(retDataType))
                                .addDependency(FunctionReturnSemantics(function), object)
                                .addTo(object)
                        ) {
                            markValueAsAffected(ctx, object);
                        }
                    }
                }
            }
        }

    private:
        void handleClassLabelFieldOperation(ResearcherPropagationContext& ctx, std::shared_ptr<ircode::Value> addrValue) {
            auto op = ctx.operation;
            auto output = op->getOutput();
            auto object = m_repository->getObject(output);
            auto linearExpr = ircode::GetLinearExpr(addrValue, true);
            auto offset = linearExpr.getConstTermValue();
            auto baseTerms = ircode::ToBaseTerms(linearExpr, getContext()->getPlatform());
            for (auto& term : baseTerms) {
                if (auto termVar = std::dynamic_pointer_cast<ircode::Variable>(term)) {
                    if (auto termObject = m_repository->getObject(termVar)) {
                        if (auto termVarInstr = termVar->getSourceOperation()->getPcodeInstruction()) {
                            ClassLabelInfo info;
                            info.structureInstrOffset = termVarInstr->getOffset();
                            info.sourceInstrOffset = op->getPcodeInstruction()->getOffset();
                            info.labelOffset = offset;
                            // TODO: if the semantic will be removed externally (by hash) we should go here
                            if (m_repository->getAdder()
                                    .add(ClassLabelFieldSemantics(info))
                                    .addDependency(OperationSemantics(op), object)
                                    .addTo(termObject)
                            ) {
                                markValueAsAffected(ctx, object);
                            }
                        }
                    }
                }
            }
        }

        void handleAddress(ResearcherPropagationContext& ctx, SemanticsObject* address) {
            auto op = ctx.operation;
            auto output = op->getOutput();
            auto object = m_repository->getObject(output);
            auto loadSize = output->getSize();
            auto isObjectAffected = false;
            for (auto holderAddrSem : address->getSemantics()) {
                if (auto symbolSem = dynamic_cast<SymbolPointerSemantics*>(holderAddrSem->getSemantics())) {
                    if (m_repository->getAdder()
                            .add(SymbolLoadSemantics(symbolSem, loadSize))
                            .addDependencyPointer(holderAddrSem)
                            .addTo(object)
                    ) {
                        isObjectAffected = true;
                    }
                    auto symbolInfo = symbolSem->getSymbolTable()->getSymbolAt(symbolSem->getOffset());
                    if (symbolInfo.symbol && symbolInfo.symbol->getOffset() == symbolInfo.requestedOffset) {
                        auto dataType = symbolInfo.symbol->getDataType();
                        if (dataType->getSize() == loadSize) {
                            if (m_repository->getAdder()
                                .add(DataTypeSemantics(dataType))
                                .addDependencyPointer(holderAddrSem)
                                .addTo(object)
                            ) {
                                isObjectAffected = true;
                            }
                        }
                    }
                }
            }
            if (isObjectAffected) {
                markValueAsAffected(ctx, object);
            }
        }

        void handleSymbolTables(ResearcherPropagationContext& ctx, const std::list<SymbolTable*>& symbolTables, size_t offset) {
            auto op = ctx.operation;
            auto output = op->getOutput();
            auto object = m_repository->getObject(output);
            auto isObjectAffected = false;
            for (auto symbolTable : symbolTables) {
                auto foundSymbols = symbolTable->getAllSymbolsRecursivelyAt(offset, true);
                for (auto [symbolTable, requestedOffset, symbol] : foundSymbols) {
                    if (m_repository->getAdder()
                        .add(SymbolPointerSemantics(symbolTable, requestedOffset))
                        .addTo(object)
                    ) {
                        isObjectAffected = true;
                    }
                }
            }
            if (isObjectAffected) {
                markValueAsAffected(ctx, object);
            }
        }

        std::list<SymbolTable*> getAllSymbolTables(ResearcherPropagationContext& ctx, std::shared_ptr<ircode::Value> value) const {
            auto op = ctx.operation;
            auto function = op->getBlock()->getFunction();
            std::list<SymbolTable*> symbolTables;
            if (auto baseRegister = ExtractRegister(value)) {
                if (baseRegister->getRegId() == Register::InstructionPointerId) {
                    symbolTables.push_back(m_program->getGlobalSymbolTable());
                } else if (baseRegister->getRegId() == Register::StackPointerId) {
                    symbolTables.push_back(function->getFunctionSymbol()->getStackSymbolTable());
                }
            }
            if (auto var = std::dynamic_pointer_cast<ircode::Variable>(value)) {
                if (auto object = m_repository->getObject(var)) {
                    for (auto holderSem : object->getSemantics()) {
                        if (auto dtSem = dynamic_cast<DataTypeSemantics*>(holderSem->getSemantics())) {
                            if (auto pointerDt = dynamic_cast<PointerDataType*>(dtSem->getDataType())) {
                                if (auto structDt = dynamic_cast<StructureDataType*>(pointerDt->getPointedType())) {
                                    symbolTables.push_back(structDt->getSymbolTable());
                                }
                            }
                        }
                    }
                }
            }
            return symbolTables;
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
                    if (m_repository->getAdder()
                            .add(DataTypeSemantics(dataType))
                            .addDependency(OperationSemantics(ctx.operation), object)
                            .addTo(object)
                    ) {
                        markValueAsAffected(ctx, object);
                    }
                }
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
        ClassRepository* m_classRepo;
        DataFlowRepository* m_dataFlowRepo;
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

            // TODO: move such event handler (working with specific semantics) to propagator
            void handleFunctionSignatureChangedEvent(const ircode::FunctionSignatureChangedEvent& event) {
                // Should be called when any change in function signature occurs (including change of data type of any parameter or return)
                ResearcherPropagationContext ctx;
                for (size_t i = 0; i < 100; ++i) {
                    auto hash = FunctionParameterSemantics(event.function, i).getHash();
                    if (auto sem = m_researcher->m_semanticsRepo->getSemantics(hash)) {
                        for (auto holder : GetSemanticsHolders(sem)) {
                            AddObjectVariablesToContext(ctx, holder);
                        }
                        m_researcher->m_semanticsRepo->removeSemanticsChain(hash);
                    } else {
                        break;
                    }
                }
                {
                    auto hash = FunctionReturnSemantics(event.function).getHash();
                    if (auto sem = m_researcher->m_semanticsRepo->getSemantics(hash)) {
                        for (auto holder : GetSemanticsHolders(sem)) {
                            AddObjectVariablesToContext(ctx, holder);
                        }
                        m_researcher->m_semanticsRepo->removeSemanticsChain(hash);
                    }
                }
                for (auto paramVar : event.function->getParamVariables()) {
                    if (!paramVar) continue;
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
                ResearcherPropagationContext ctx;
                auto output = event.op->getOutput();
                if (auto outputObject = m_researcher->m_semanticsRepo->getObject(output)) {
                    markValueAsAffected(ctx, outputObject);
                    m_researcher->m_semanticsRepo->unbindVariableWithObject(output, outputObject);
                    if (outputObject->getVariables().empty()) {
                        m_researcher->m_semanticsRepo->removeObject(outputObject);
                    } else {
                        m_researcher->m_semanticsRepo->cleanObject(outputObject);
                    }
                }
                ctx.collect([&]() {
                    m_researcher->propagate(ctx);
                });
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
                            for (auto node : structure->linkedNodes) {
                                if (auto var = node->getVariable()) {
                                    if (auto object = m_researcher->m_semanticsRepo->getObject(var)) {
                                        objectsToRemove.insert(object);
                                    }
                                }
                            }
                        }
                    }
                    // find new variables for each offset and label
                    std::map<std::pair<size_t, size_t>, std::list<std::shared_ptr<ircode::Variable>>> labelAndOffsetToVar;
                    for (auto structure : structuresInGroup) {
                        auto info = m_researcher->m_classRepo->getStructureInfo(structure);
                        auto labelHash = labelsToHash(info->labels);
                        for (auto node : structure->linkedNodes) {
                            if (auto var = node->getVariable()) {
                                labelAndOffsetToVar[std::pair(labelHash, node->offset)].push_back(var);
                            }
                        }
                    }
                    for (auto& [_, vars] : labelAndOffsetToVar) {
                        auto& group = groupsOfIdenticVariables.emplace_back();
                        group.insert(group.end(), vars.begin(), vars.end());
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
                    markValueAsAffected(ctx, object);
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
                // Notice: one semantics object can be shared by several structures
                ResearcherPropagationContext ctx;
                ResearcherPropagationContext ctx2;
                if (event.structure->sourceNode) {
                    if (auto sourceVar = event.structure->sourceNode->getVariable()) {
                        if (auto object = m_researcher->m_semanticsRepo->getObject(sourceVar)) {
                            markValueAsAffected(ctx, object);
                            for (auto node : event.structure->linkedNodes) {
                                if (auto var = node->getVariable()) {
                                    // remove structure's variables from object
                                    m_researcher->m_semanticsRepo->unbindVariableWithObject(var, object);
                                    ctx2.addNextOperation(var->getSourceOperation());
                                }
                            }
                            // clean or remove semantic object
                            if (object->getVariables().empty()) {
                                // if objects belongs to single structure, remove it
                                m_researcher->m_semanticsRepo->removeObject(object);
                            } else {
                                m_researcher->m_semanticsRepo->cleanObject(object);
                            }
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

            void handleSymbolPointerUpdatedEvent(SymbolTable* symbolTable, Offset startOffset, size_t size) {
                ResearcherPropagationContext ctx;
                for (size_t offset = startOffset; offset < startOffset + size; ++offset) {
                    auto symbolSemHash = SymbolPointerSemantics(symbolTable, offset).getHash();
                    if (auto symbolSem = m_researcher->m_semanticsRepo->getSemantics(symbolSemHash)) {
                        for (auto holder : GetSemanticsHolders(symbolSem)) {
                            AddObjectVariablesToContext(ctx, holder);
                        }
                        m_researcher->m_semanticsRepo->removeSemanticsChain(symbolSemHash);
                    }
                }
                ctx.collect([&]() {
                    m_researcher->propagate(ctx);
                });
            }
            
            void handleObjectModifiedEvent(const ObjectModifiedEvent& event) {
                if (event.state == Object::ModState::Before) {
                    return;
                }
                if (auto symbol = dynamic_cast<Symbol*>(event.object)) {
                    if (auto symbolTable = symbol->getSymbolTable()) {
                        handleSymbolPointerUpdatedEvent(symbolTable, symbol->getOffset(), symbol->getDataType()->getSize());
                    }
                }
            }

            void handleObjectRemovedEvent(const ObjectRemovedEvent& event) {
                ResearcherPropagationContext ctx;
                if (auto dataType = dynamic_cast<DataType*>(event.object)) {
                    auto hash = DataTypeSemantics(dataType).getHash();
                    if (auto sem = m_researcher->m_semanticsRepo->getSemantics(hash)) {
                        for (auto holder : GetSemanticsHolders(sem)) {
                            AddObjectVariablesToContext(ctx, holder);
                        }
                        m_researcher->m_semanticsRepo->removeSemanticsChain(hash);
                    }
                }
                ctx.collect([&]() {
                    // TODO optimization: join it with other events (SymbolPointerUpdatedEvent, ...) that emitted most together
                    m_researcher->propagate(ctx);
                });
            }

            void handleSymbolTableSymbolRemovedEvent(const SymbolTableSymbolRemovedEvent& event) {
                handleSymbolPointerUpdatedEvent(event.symbolTable, event.offset, event.symbol->getDataType()->getSize());
            }

            size_t labelsToHash(const std::set<size_t>& labels) {
                size_t hash = 0;
                for (auto label : labels) {
                    boost::hash_combine(hash, label);
                }
                return hash;
            }

            void markValueAsAffected(ResearcherPropagationContext& ctx, SemanticsObject* object) {
                MarkObjectAsAffected(ctx, object, m_researcher->m_dataFlowRepo);
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
                pipe->subscribeMethod(this, &EventHandler::handleObjectModifiedEvent);
                pipe->subscribeMethod(this, &EventHandler::handleObjectRemovedEvent);
                pipe->subscribeMethod(this, &EventHandler::handleSymbolTableSymbolRemovedEvent);
                return pipe;
            }
        };
        EventHandler m_eventHandler;
    public:
        SemanticsResearcher(
            ircode::Program* program,
            SemanticsRepository* semanticsRepo,
            ClassRepository* classRepo,
            DataFlowRepository* dataFlowRepo
        )
            : m_program(program)
            , m_semanticsRepo(semanticsRepo)
            , m_classRepo(classRepo)
            , m_dataFlowRepo(dataFlowRepo)
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