#pragma once
#include "Semantics.h"
#include "SDA/Core/IRcode/IRcodeBlock.h"
#include "SDA/Core/IRcode/IRcodeEvents.h"
#include "SDA/Core/Utils/Logger.h"
#include "SDA/Core/Utils/IOManip.h"

namespace sda::semantics
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

        std::string getName(bool full = true) {
            if (!value) {
                return "Start";
            }
            else if (auto constant = std::dynamic_pointer_cast<ircode::Constant>(value)) {
                return (std::stringstream() << "0x" << utils::to_hex() << constant->getConstVarnode()->getValue()).str();
            }
            else if (auto var = std::dynamic_pointer_cast<ircode::Variable>(value)) {
                return var->getName(full);
            }
            throw std::runtime_error("Unknown value type");
        }

        static void PassSuccessors(const std::list<DataFlowNode*>& startNodes, std::function<void(DataFlowNode* node, bool& goNextNodes)> callback) {
            std::map<DataFlowNode*, size_t> nodeKnocks;
            std::list<DataFlowNode*> nodesToVisit;
            for (auto startNode : startNodes) {
                nodesToVisit.push_back(startNode);
            }
            do {
                while (!nodesToVisit.empty()) {
                    auto node = nodesToVisit.front();
                    nodesToVisit.pop_front();
                    auto it = nodeKnocks.find(node);
                    if (it == nodeKnocks.end()) {
                        it = nodeKnocks.insert({ node, 0 }).first;
                    }
                    auto knocks = ++it->second;
                    if (knocks < node->predecessors.size()) {
                        continue;
                    }
                    nodeKnocks.erase(it);
                    bool goNextNodes = false;
                    callback(node, goNextNodes);
                    if (goNextNodes) {
                        for (auto nextNode : node->successors) {
                            nodesToVisit.push_back(nextNode);
                        }
                    }
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

        DataFlowNode* getOrCreateNode(std::shared_ptr<ircode::Value> value) {
            auto node = getNode(value);
            if (!node) {
                m_nodes[value] = { DataFlowNode::Unknown, value };
                auto newNode = &m_nodes[value];
                m_eventPipe->send(DataFlowNodeCreatedEvent(newNode));
                return newNode;
            }
            return node;
        }

        DataFlowNode* createNode(DataFlowNode::Type type, std::shared_ptr<ircode::Value> value, Offset offset = 0) {
            auto node = getOrCreateNode(value);
            if (node->type == DataFlowNode::Unknown) {
                node->type = type;
                node->offset = offset;
                m_eventPipe->send(DataFlowNodeUpdatedEvent(node));
                return node;
            }
            return nullptr;
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

        void addSuccessor(DataFlowNode* node, DataFlowNode* successor) {
            auto& successors = node->successors;
            auto it = std::find(successors.begin(), successors.end(), successor);
            if (it != successors.end()) {
                return;
            }
            successors.push_back(successor);
            successor->predecessors.push_back(node);
            m_eventPipe->send(DataFlowNodeUpdatedEvent(successor));
        }
    };

    class DataFlowCollector
    {
        ircode::Program* m_program;
        Platform* m_platform;
        DataFlowRepository* m_dataFlowRepo;

        class IRcodeEventHandler
        {
            DataFlowCollector* m_collector;

            void handleFunctionDecompiled(const ircode::FunctionDecompiledEvent& event) {
                SemanticsPropagationContext ctx;
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
            Platform* platform,
            DataFlowRepository* dataFlowRepo
        )
            : m_program(program)
            , m_platform(platform)
            , m_dataFlowRepo(dataFlowRepo)
            , m_ircodeEventHandler(this)
        {}

        std::shared_ptr<EventPipe> getEventPipe() {
            return m_ircodeEventHandler.getEventPipe();
        }

        void research(SemanticsPropagationContext& ctx)
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
                            if (auto copyNode = m_dataFlowRepo->createNode(DataFlowNode::Copy, output)) {
                                m_dataFlowRepo->addSuccessor(startNode, copyNode);
                            }
                        }
                    } else {
                        // variable or constant
                        if (auto inputNode = m_dataFlowRepo->getOrCreateNode(input)) {
                            if (auto readNode = m_dataFlowRepo->createNode(DataFlowNode::Read, output)) {
                                m_dataFlowRepo->addSuccessor(inputNode, readNode);
                                ctx.markValueAsAffected(input);
                            }
                        }
                    }
                }
                else if (unaryOp->getId() == ircode::OperationId::COPY || unaryOp->getId() == ircode::OperationId::REF) {
                    if (auto inputNode = m_dataFlowRepo->getOrCreateNode(input)) {
                        auto outputAddrVal = output->getMemAddress().value;
                        if (auto outputAddrVar = std::dynamic_pointer_cast<ircode::Variable>(outputAddrVal)) {
                            if (auto addrNode = m_dataFlowRepo->getOrCreateNode(outputAddrVar)) {
                                if (auto writeNode = m_dataFlowRepo->createNode(DataFlowNode::Write, output)) {
                                    m_dataFlowRepo->addSuccessor(addrNode, writeNode);
                                    m_dataFlowRepo->addSuccessor(inputNode, writeNode);
                                    ctx.markValueAsAffected(outputAddrVar);
                                    ctx.markValueAsAffected(input);
                                }
                            }
                        } else {
                            if (auto copyNode = m_dataFlowRepo->createNode(DataFlowNode::Copy, output)) {
                                m_dataFlowRepo->addSuccessor(inputNode, copyNode);
                                ctx.markValueAsAffected(input);
                            }
                        }
                    }
                }
            }
            else if (auto binaryOp = dynamic_cast<const ircode::BinaryOperation*>(ctx.operation)) {
                if (binaryOp->getId() == ircode::OperationId::INT_ADD ||
                    binaryOp->getId() == ircode::OperationId::INT_MULT)
                {
                    auto linearExpr = ircode::Value::GetLinearExpr(output);
                    Offset offset = linearExpr.getConstTermValue();
                    for (auto& term : linearExpr.getTerms()) {
                        if (term.factor != 1 || term.value->getSize() != m_platform->getPointerSize())
                            continue;
                        if (auto termVar = std::dynamic_pointer_cast<ircode::Variable>(term.value)) {
                            if (m_dataFlowRepo->getNode(output) || m_dataFlowRepo->getNode(termVar)) {
                                if (auto inputNode = m_dataFlowRepo->getOrCreateNode(termVar)) {
                                    if (auto copyNode = m_dataFlowRepo->createNode(DataFlowNode::Copy, output, offset)) {
                                        m_dataFlowRepo->addSuccessor(inputNode, copyNode);
                                        ctx.markValueAsAffected(termVar);
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
                            if (auto copyNode = m_dataFlowRepo->createNode(DataFlowNode::Copy, output)) {
                                m_dataFlowRepo->addSuccessor(inputNode1, copyNode);
                                m_dataFlowRepo->addSuccessor(inputNode2, copyNode);
                                ctx.markValueAsAffected(binaryOp->getInput1());
                                ctx.markValueAsAffected(binaryOp->getInput2());
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
                    if (auto paramNode = m_dataFlowRepo->getOrCreateNode(paramVar)) {
                        m_dataFlowRepo->addSuccessor(argNode, paramNode);
                    }
                }
            }
            if (auto returnVar = function->getReturnVariable()) {
                if (auto outputNode = m_dataFlowRepo->getOrCreateNode(callOp->getOutput())) {
                    if (auto returnNode = m_dataFlowRepo->getOrCreateNode(returnVar)) {
                        m_dataFlowRepo->addSuccessor(returnNode, outputNode);
                    }
                }
            }
        }
    };
};