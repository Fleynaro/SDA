#pragma once
#include "ResearcherHelper.h"
#include "SDA/Core/IRcode/IRcodeBlock.h"
#include "SDA/Core/IRcode/IRcodeEvents.h"
#include "SDA/Core/IRcode/IRcodeHelper.h"
#include "SDA/Core/Utils/Logger.h"
#include "SDA/Core/Utils/String.h"
#include "SDA/Core/Utils/AbstractPrinter.h"

namespace sda::researcher
{
    static const size_t DataFlowEventTopic = TopicName("DataFlowEventTopic");

    struct DataFlowNode;

    // When a data flow node is created
    struct DataFlowNodeCreatedEvent : Event {
        DataFlowNode* node;

        DataFlowNodeCreatedEvent(DataFlowNode* node)
            : Event(DataFlowEventTopic)
            , node(node)
        {}
    };

    // When data flow node predecessor is updated
    struct DataFlowNodeUpdatedEvent : Event {
        DataFlowNode* node;

        DataFlowNodeUpdatedEvent(DataFlowNode* node)
            : Event(DataFlowEventTopic)
            , node(node)
        {}
    };

    // When a data flow node is removed
    struct DataFlowNodeRemovedEvent : Event {
        DataFlowNode* node;

        DataFlowNodeRemovedEvent(DataFlowNode* node)
            : Event(DataFlowEventTopic)
            , node(node)
        {}
    };

    // When a node is researched
    struct DataFlowNodePassedEvent : Event {
        DataFlowNode* node;
        std::list<DataFlowNode*> nextNodes;

        DataFlowNodePassedEvent(DataFlowNode* node, const std::list<DataFlowNode*>& nextNodes)
            : Event(DataFlowEventTopic)
            , node(node)
            , nextNodes(nextNodes)
        {}
    };

    struct DataFlowNode {
        enum Type {
            Unknown,
            Start,
            Copy,
            Write,
            Read
        };
        Type type = Unknown;
        std::shared_ptr<ircode::Value> value;
        Offset offset = 0;
        std::list<DataFlowNode*> predecessors;
        std::list<DataFlowNode*> successors;

        size_t getVarPredecessorCount() {
            size_t count = 0;
            for (auto& pred : predecessors) {
                if (pred->getVariable()) {
                    ++count;
                }
            }
            return count;
        }

        const std::list<DataFlowNode*>& getPredecessors() {
            return predecessors;
        }

        const std::list<DataFlowNode*>& getSuccessors() {
            return successors;
        }

        std::shared_ptr<ircode::Variable> getVariable() {
            if (auto var = std::dynamic_pointer_cast<ircode::Variable>(value)) {
                return var;
            }
            return nullptr;
        }

        std::shared_ptr<ircode::Constant> getConstant() {
            if (auto constant = std::dynamic_pointer_cast<ircode::Constant>(value)) {
                return constant;
            }
            return nullptr;
        }

        Type getType() {
            return type;
        }

        std::string getName(bool full = true) {
            if (!value) {
                return "Start";
            }
            else if (auto constant = std::dynamic_pointer_cast<ircode::Constant>(value)) {
                return "0x" + utils::ToHex(constant->getConstVarnode()->getValue());
            }
            else if (auto var = std::dynamic_pointer_cast<ircode::Variable>(value)) {
                return var->getName(full);
            }
            throw std::runtime_error("Unknown value type");
        }

        static void PassSuccessors(
            DataFlowNode* startNode,
            std::function<void(DataFlowNode* node, bool& goNextNodes, const std::function<void(DataFlowNode* node)>& next)> callback,
            std::shared_ptr<EventPipe> eventPipe = nullptr)
        {
            std::map<DataFlowNode*, size_t> nodeKnocks;
            std::list<DataFlowNode*> nodesToVisit;
            nodesToVisit.push_back(startNode);
            do {
                while (!nodesToVisit.empty()) {
                    auto node = nodesToVisit.front();
                    nodesToVisit.pop_front();
                    auto it = nodeKnocks.find(node);
                    if (it == nodeKnocks.end()) {
                        it = nodeKnocks.insert({ node, 0 }).first;
                    }
                    auto knocks = ++it->second;
                    if (knocks < node->getVarPredecessorCount()) {
                        continue;
                    }
                    nodeKnocks.erase(it);
                    bool goNextNodes = false;
                    auto nextNodes = node->successors;
                    auto next = [&](DataFlowNode* node) {
                        nextNodes.push_front(node);
                    };
                    callback(node, goNextNodes, next);
                    if (goNextNodes) {
                        for (auto it = nextNodes.rbegin(); it != nextNodes.rend(); ++it) {
                            nodesToVisit.push_front(*it);
                        }
                    }
                    eventPipe->send(DataFlowNodePassedEvent(node, nodesToVisit));
                }
                if (!nodeKnocks.empty()) {
                    auto node = nodeKnocks.begin()->first;
                    nodesToVisit.push_back(node);
                }
            } while (!nodesToVisit.empty());
        }
    };

    class DataFlowRepository
    {
        std::map<std::shared_ptr<ircode::Value>, DataFlowNode> m_nodes;
        DataFlowNode m_globalNode;
        std::shared_ptr<EventPipe> m_eventPipe;
    public:
        DataFlowRepository(std::shared_ptr<EventPipe> eventPipe);

        DataFlowNode* getGlobalStartNode() {
            return &m_globalNode;
        }

        DataFlowNode* getNode(std::shared_ptr<ircode::Value> value) {
            auto it = m_nodes.find(value);
            if (it == m_nodes.end()) {
                return nullptr;
            }
            return &it->second;
        }

        DataFlowNode* getOrCreateNode(std::shared_ptr<ircode::Value> value, DataFlowNode::Type type = DataFlowNode::Unknown, Offset offset = 0) {
            auto node = getNode(value);
            if (!node) {
                m_nodes[value] = { type, value, offset };
                auto newNode = &m_nodes[value];
                m_eventPipe->send(DataFlowNodeCreatedEvent(newNode));
                return newNode;
            } else {
                if (node->type == DataFlowNode::Unknown) {
                    node->type = type;
                    node->offset = offset;
                    m_eventPipe->send(DataFlowNodeUpdatedEvent(node));
                }
            }
            return node;
        }

        void removeNode(DataFlowNode* node) {
            m_eventPipe->send(DataFlowNodeRemovedEvent(node));
            for (auto pred : node->predecessors) {
                pred->successors.remove(node);
            }
            for (auto succ : node->successors) {
                succ->predecessors.remove(node);
            }
            m_nodes.erase(node->value);
        }

        bool addSuccessor(DataFlowNode* node, DataFlowNode* successor) {
            auto& successors = node->successors;
            auto it = std::find(successors.begin(), successors.end(), successor);
            if (it != successors.end()) {
                return false;
            }
            successors.push_back(successor);
            successor->predecessors.push_back(node);
            m_eventPipe->send(DataFlowNodeUpdatedEvent(successor));
            return true;
        }
    };

    class DataFlowCollector
    {
        ircode::Program* m_program;
        DataFlowRepository* m_dataFlowRepo;

        class IRcodeEventHandler
        {
            DataFlowCollector* m_collector;

            void handleFunctionDecompiled(const ircode::FunctionDecompiledEvent& event) {
                ResearcherPropagationContext ctx;
                for (auto block : event.blocks) { 
                    for (auto& op : block->getOperations()) {
                        ctx.addNextOperation(op.get());
                    }
                }
                ctx.collect([&]() {
                    m_collector->research(ctx);
                });
                // update data flow nodes in other function because some vars can be removed in this function
                auto callOps = m_collector->m_program->getCallsRefToFunction(event.function);
                for (auto callOp : callOps) {
                    m_collector->researchCallOperation(callOp, event.function);
                }
            }

            void handleOperationRemoved(const ircode::OperationRemovedEvent& event) {
                auto output = event.op->getOutput();
                if (auto node = m_collector->m_dataFlowRepo->getNode(output)) {
                    m_collector->m_dataFlowRepo->removeNode(node);
                }
            }
        public:
            IRcodeEventHandler(DataFlowCollector* collector) : m_collector(collector) {}

            std::shared_ptr<EventPipe> getEventPipe() {
                auto pipe = EventPipe::New();
                pipe->subscribeMethod(this, &IRcodeEventHandler::handleFunctionDecompiled);
                pipe->subscribeMethod(this, &IRcodeEventHandler::handleOperationRemoved);
                return pipe;
            }
        };
        IRcodeEventHandler m_ircodeEventHandler;
    public:
        DataFlowCollector(
            ircode::Program* program,
            DataFlowRepository* dataFlowRepo
        )
            : m_program(program)
            , m_dataFlowRepo(dataFlowRepo)
            , m_ircodeEventHandler(this)
        {}

        std::shared_ptr<EventPipe> getEventPipe() {
            return m_ircodeEventHandler.getEventPipe();
        }

        void research(ResearcherPropagationContext& ctx)
        {
            auto output = ctx.operation->getOutput();
            if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(ctx.operation)) {
                auto input = unaryOp->getInput();
                if (unaryOp->getId() == ircode::OperationId::LOAD) {
                    if (auto inputReg = std::dynamic_pointer_cast<ircode::Register>(input)) {
                        // register
                        auto regId = inputReg->getRegister().getRegId();
                        if (regId == Register::InstructionPointerId) {
                            auto startNode = m_dataFlowRepo->getGlobalStartNode();
                            if (auto copyNode = m_dataFlowRepo->getOrCreateNode(output, DataFlowNode::Copy)) {
                                m_dataFlowRepo->addSuccessor(startNode, copyNode);
                            }
                        }
                    } else {
                        // variable or constant
                        if (auto inputNode = m_dataFlowRepo->getOrCreateNode(input)) {
                            if (auto readNode = m_dataFlowRepo->getOrCreateNode(output, DataFlowNode::Read)) {
                                m_dataFlowRepo->addSuccessor(inputNode, readNode);
                            }
                        }
                    }
                }
                else if (unaryOp->getId() == ircode::OperationId::COPY || unaryOp->getId() == ircode::OperationId::REF) {
                    if (auto inputNode = m_dataFlowRepo->getOrCreateNode(input)) {
                        auto outputAddrVal = output->getMemAddress().value;
                        auto outputAddrVar = std::dynamic_pointer_cast<ircode::Variable>(outputAddrVal);
                        if (outputAddrVar && !output->getMemAddress().isVirtual) {
                            if (auto addrNode = m_dataFlowRepo->getOrCreateNode(outputAddrVar)) {
                                if (auto writeNode = m_dataFlowRepo->getOrCreateNode(output, DataFlowNode::Write)) {
                                    if (m_dataFlowRepo->addSuccessor(addrNode, writeNode)) {
                                        // revise outputAddrVar because it's already known its type is address
                                        ctx.markValueAsAffected(outputAddrVar);
                                    }
                                    m_dataFlowRepo->addSuccessor(inputNode, writeNode);
                                }
                            }
                        } else {
                            if (auto copyNode = m_dataFlowRepo->getOrCreateNode(output, DataFlowNode::Copy)) {
                                m_dataFlowRepo->addSuccessor(inputNode, copyNode);
                            }
                        }
                    }
                }
            }
            else if (auto binaryOp = dynamic_cast<const ircode::BinaryOperation*>(ctx.operation)) {
                if (binaryOp->getId() == ircode::OperationId::INT_ADD ||
                    binaryOp->getId() == ircode::OperationId::INT_MULT)
                {
                    auto linearExpr = ircode::GetLinearExpr(output);
                    auto offset = linearExpr.getConstTermValue();
                    auto baseTerms = ircode::ToBaseTerms(linearExpr, m_program->getPlatform());
                    for (auto& term : baseTerms) {
                        if (auto termVar = std::dynamic_pointer_cast<ircode::Variable>(term)) {
                            // if one of the nodes is already created, then it is address, not just another linear expression
                            if (m_dataFlowRepo->getNode(output) || m_dataFlowRepo->getNode(termVar)) {
                                if (auto inputNode = m_dataFlowRepo->getOrCreateNode(termVar)) {
                                    if (auto copyNode = m_dataFlowRepo->getOrCreateNode(output, DataFlowNode::Copy, offset)) {
                                        m_dataFlowRepo->addSuccessor(inputNode, copyNode);
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
                else if (binaryOp->getId() == ircode::OperationId::PHI) {
                    if (auto inputNode1 = m_dataFlowRepo->getOrCreateNode(binaryOp->getInput1())) {
                        if (auto inputNode2 = m_dataFlowRepo->getOrCreateNode(binaryOp->getInput2())) {
                            if (auto copyNode = m_dataFlowRepo->getOrCreateNode(output, DataFlowNode::Copy)) {
                                m_dataFlowRepo->addSuccessor(inputNode1, copyNode);
                                m_dataFlowRepo->addSuccessor(inputNode2, copyNode);
                            }
                        }
                    }
                }
            }
            else if (auto callOp = dynamic_cast<const ircode::CallOperation*>(ctx.operation)) {
                auto functions = m_program->getFunctionsByCallInstruction(callOp->getPcodeInstruction());
                for (auto function : functions) {
                    researchCallOperation(callOp, function);
                }
            }
        }

        void researchCallOperation(const ircode::CallOperation* callOp, ircode::Function* function)
        {
            auto& argValues = callOp->getArguments();
            auto& paramsVars = function->getParamVariables();
            assert(argValues.size() == paramsVars.size());
            for (size_t i = 0; i < argValues.size(); ++i) {
                auto argValue = argValues[i];
                auto paramVar = paramsVars[i];
                if (auto argNode = m_dataFlowRepo->getOrCreateNode(argValue)) {
                    if (auto paramNode = m_dataFlowRepo->getOrCreateNode(paramVar, DataFlowNode::Copy)) {
                        m_dataFlowRepo->addSuccessor(argNode, paramNode);
                    }
                }
            }
            if (auto returnVar = function->getReturnVariable()) {
                if (auto outputNode = m_dataFlowRepo->getOrCreateNode(callOp->getOutput(), DataFlowNode::Copy)) {
                    if (auto returnNode = m_dataFlowRepo->getOrCreateNode(returnVar)) {
                        m_dataFlowRepo->addSuccessor(returnNode, outputNode);
                    }
                }
            }
        }
    };

    static std::string GetDataFlowNodeName(researcher::DataFlowNode* node, ircode::Function* function) {
        if (auto var = node->getVariable()) {
            auto varFunction = var->getSourceOperation()->getBlock()->getFunction();
            return node->getName(varFunction != function);
        }
        return node->getName();
    }

    static std::string PrintDataFlowForFunction(researcher::DataFlowRepository* dataFlowRepo, ircode::Function* function) {
        std::stringstream ss;
        utils::AbstractPrinter printer;
        printer.setOutput(ss);
        for (size_t i = 0; i < 2; ++i)
            printer.startBlock();
        printer.newTabs();
        // create list of current variables (of current function) and external variables (of referred functions)
        auto variables = function->getVariables();
        std::list<std::shared_ptr<sda::ircode::Variable>> extVariables;
        for (auto var : variables) {
            if (auto node = dataFlowRepo->getNode(var)) {
                for (auto succNode : node->successors) {
                    if (auto variable = succNode->getVariable()) {
                        if (std::find(variables.begin(), variables.end(), variable) != variables.end())
                            continue;
                        if (std::find(extVariables.begin(), extVariables.end(), variable) != extVariables.end())
                            continue;
                        extVariables.push_back(variable);
                    }
                }
            }
        }
        for (auto vars : { &variables, &extVariables }) {
            vars->sort([](std::shared_ptr<ircode::Variable> var1, std::shared_ptr<ircode::Variable> var2) {
                auto func1 = var1->getSourceOperation()->getBlock()->getFunction();
                auto func2 = var2->getSourceOperation()->getBlock()->getFunction();
                if (func1 != func2)
                    return func1->getName() < func2->getName();
                return var1->getId() < var2->getId();
            });
        }
        variables.insert(variables.end(), extVariables.begin(), extVariables.end());
        // output
        for (auto var : variables) {
            if (auto node = dataFlowRepo->getNode(var)) {
                if (node->predecessors.empty()) {
                    ss << GetDataFlowNodeName(node, function) << " <- Unknown";
                    printer.newLine();
                } else {
                    for (auto predNode : node->predecessors) {
                        if (auto predVar = predNode->getVariable()) {
                            if (var->getSourceOperation()->getBlock()->getFunction() != function && 
                                predVar->getSourceOperation()->getBlock()->getFunction() != function) continue;
                        }
                        ss << GetDataFlowNodeName(node, function) << " <- ";
                        if (node->type == researcher::DataFlowNode::Copy)
                            ss << "Copy ";
                        else if (node->type == researcher::DataFlowNode::Write)
                            ss << "Write ";
                        else if (node->type == researcher::DataFlowNode::Read)
                            ss << "Read ";
                        ss << GetDataFlowNodeName(predNode, function);
                        if (node->offset > 0) {
                            ss << " + 0x" << utils::ToHex(node->offset);
                        }
                        printer.newLine();
                    }
                }
            }
        }
        return ss.str();
    }
};