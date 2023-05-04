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

    class MemoryStructureSemanticsRepository : public SemanticsRepository
    {
        struct Node {
            enum Type {
                Start,
                AddOffset,
                Load
            };
            Type type;
            Offset offset;
            std::list<std::shared_ptr<ircode::Variable>> variables;
            std::list<Node*> predecessors;
            std::list<Node*> successors;
        };
        std::list<Node> m_nodes;
        std::map<std::shared_ptr<ircode::Variable>, Node*> m_varToNode;
        Node* m_globalNode;
    public:
        MemoryStructureSemanticsRepository(SemanticsManager* manager)
            : SemanticsRepository(manager)
        {
            m_globalNode = createNode(Node::Start);
        }

        Node* getGlobalStartNode() const {
            return m_globalNode;
        }

        Node* getNode(std::shared_ptr<ircode::Variable> variable) {
            auto it = m_varToNode.find(variable);
            if (it == m_varToNode.end()) {
                return nullptr;
            }
            return it->second;
        }

        std::list<std::shared_ptr<ircode::Variable>> getSameVariables(std::shared_ptr<ircode::Variable> variable) {
            auto node = getNode(variable);
            return node->variables;
        }

        Node* addStartVariable(Node* node, std::shared_ptr<ircode::Variable> variable) {
            return addVariable(node, variable);
        }

        Node* addCopyVariable(Node* node, std::shared_ptr<ircode::Variable> variable) {
            return addVariable(node, variable);
        }

        Node* addLoadVariable(Node* parentNode, std::shared_ptr<ircode::Variable> variable) {
            return addVariable(parentNode, variable, Node::Load);
        }

        Node* addOffsetVariable(Node* parentNode, std::shared_ptr<ircode::Variable> variable, Offset offset) {
            return addVariable(parentNode, variable, Node::AddOffset, offset);
        }

        void addSuccessor(Node* node, Node* successor) {
            node->successors.push_back(successor);
            successor->predecessors.push_back(node);
        }

    private:
        Node* addVariable(
            Node* parentNode,
            std::shared_ptr<ircode::Variable> variable,
            Node::Type type,
            Offset offset = 0)
        {
            auto successor = findSuccessor(parentNode, type, offset);
            if (!successor) {
                successor = createNode(type, offset);
                addSuccessor(parentNode, successor);
            }
            return addVariable(successor, variable);
        }

        Node* addVariable(Node* node, std::shared_ptr<ircode::Variable> variable) {
            auto existingNode = getNode(variable);
            if (existingNode) {
                return nullptr;
            }
            node->variables.push_back(variable);
            m_varToNode[variable] = node;
            return node;
        }

        Node* createNode(Node::Type type, Offset offset = 0) {
            m_nodes.push_back({ type, offset });
            return &m_nodes.back();
        }

        Node* findSuccessor(Node* node, Node::Type type, Offset offset = 0) {
            // TODO: optimize (use map)
            for (auto successor : node->successors) {
                if (successor->type == type && successor->offset == offset) {
                    return successor;
                }
            }
            return nullptr;
        }
    };

    class MemoryStructureSemanticsPropagator : public SemanticsPropagator
    {
        Platform* m_platform;
        MemoryStructureSemanticsRepository* m_memStructRepo;
    public:
        MemoryStructureSemanticsPropagator(
            Platform* platform,
            MemoryStructureSemanticsRepository* memStructRepo
        )
            : m_platform(platform)
            , m_memStructRepo(memStructRepo)
        {}

        void propagate(SemanticsPropagationContext& ctx) override
        {
            auto output = ctx.operation->getOutput();
            if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(ctx.operation)) {
                auto input = unaryOp->getInput();
                if (unaryOp->getId() == ircode::OperationId::LOAD) {
                    if (auto inputReg = std::dynamic_pointer_cast<ircode::Register>(input)) {
                        auto regId = inputReg->getRegister().getRegId();
                        if (regId == Register::InstructionPointerId) {
                            auto startNode = m_memStructRepo->getGlobalStartNode();
                            if (m_memStructRepo->addStartVariable(startNode, output)) {
                                ctx.markValueAsAffected(output);
                            }
                        }
                    }
                    else if (auto inputVar = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                        if (auto node = m_memStructRepo->getNode(inputVar)) {
                            if (m_memStructRepo->addLoadVariable(node, output)) {
                                ctx.markValueAsAffected(output);
                            }
                        }
                    }
                }
                else if (unaryOp->getId() == ircode::OperationId::COPY) {
                    if (auto inputVar = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                        if (auto node = m_memStructRepo->getNode(inputVar)) {
                            if (m_memStructRepo->addCopyVariable(node, output)) {
                                ctx.markValueAsAffected(output);
                            }
                        }
                        auto outputAddrVal = output->getMemAddress().value;
                        if (auto outputAddrVar = std::dynamic_pointer_cast<ircode::Variable>(outputAddrVal)) {
                            if (auto addrNode = m_memStructRepo->getNode(outputAddrVar)) {
                                if (auto inputNode = m_memStructRepo->getNode(inputVar)) {
                                    m_memStructRepo->addSuccessor(addrNode, inputNode);
                                } else {
                                    if (auto loadNode = m_memStructRepo->addLoadVariable(addrNode, output)) {
                                        if (m_memStructRepo->addCopyVariable(loadNode, inputVar)) {
                                            ctx.markValueAsAffected(inputVar);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else if (auto binaryOp = dynamic_cast<const ircode::BinaryOperation*>(ctx.operation)) {
                if (binaryOp->getId() == ircode::OperationId::INT_ADD ||
                    binaryOp->getId() == ircode::OperationId::INT_MULT)
                {
                    auto linearExpr = output->getLinearExpr();
                    Offset offset = linearExpr.getConstTermValue();
                    for (auto& term : linearExpr.getTerms()) {
                        if (term.factor != 1 || term.value->getSize() != m_platform->getPointerSize())
                            continue;
                        if (auto termVar = std::dynamic_pointer_cast<ircode::Variable>(term.value)) {
                            if (auto node = m_memStructRepo->getNode(termVar)) {
                                if (m_memStructRepo->addOffsetVariable(node, output, offset)) {
                                    ctx.markValueAsAffected(output);
                                }
                                break;
                            }
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
