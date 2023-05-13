#pragma once
#include "Semantics.h"

namespace sda::semantics
{
    struct DataFlowNode {
        enum Type {
            Unknown,
            Start,
            Copy,
            Write,
            Read
        };
        Type type = Unknown;
        std::shared_ptr<ircode::Variable> variable;
        Offset offset = 0;
        std::list<DataFlowNode*> predecessors;
        std::list<DataFlowNode*> successors;
    };

    class DataFlowSemanticsRepository : public SemanticsRepository
    {
        std::map<std::shared_ptr<ircode::Variable>, DataFlowNode> m_nodes;
        DataFlowNode m_globalNode;
    public:
        DataFlowSemanticsRepository(SemanticsManager* manager)
            : SemanticsRepository(manager)
        {
            m_globalNode = { DataFlowNode::Start };
        }

        DataFlowNode* getGlobalStartNode() {
            return &m_globalNode;
        }

        DataFlowNode* getNode(std::shared_ptr<ircode::Variable> variable) {
            auto it = m_nodes.find(variable);
            if (it == m_nodes.end()) {
                return nullptr;
            }
            return &it->second;
        }

        DataFlowNode* getOrCreateNode(std::shared_ptr<ircode::Variable> variable) {
            auto node = getNode(variable);
            if (!node) {
                m_nodes[variable] = { DataFlowNode::Unknown, variable };
                return &m_nodes[variable];
            }
            return node;
        }

        DataFlowNode* createNode(DataFlowNode::Type type, std::shared_ptr<ircode::Variable> variable, Offset offset = 0) {
            auto node = getOrCreateNode(variable);
            if (node->type == DataFlowNode::Unknown) {
                node->type = type;
                node->offset = offset;
                return node;
            }
            return nullptr;
        }

        void addSuccessor(DataFlowNode* node, DataFlowNode* successor) {
            node->successors.push_back(successor);
            successor->predecessors.push_back(node);
        }
    };

    class DataFlowSemanticsPropagator : public SemanticsPropagator
    {
        Platform* m_platform;
        DataFlowSemanticsRepository* m_dataFlowRepo;
    public:
        DataFlowSemanticsPropagator(
            Platform* platform,
            DataFlowSemanticsRepository* dataFlowRepo
        )
            : m_platform(platform)
            , m_dataFlowRepo(dataFlowRepo)
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
                            auto startNode = m_dataFlowRepo->getGlobalStartNode();
                            if (auto copyNode = m_dataFlowRepo->createNode(DataFlowNode::Copy, output)) {
                                m_dataFlowRepo->addSuccessor(startNode, copyNode);
                            }
                        }
                    }
                    else if (auto inputVar = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                        if (auto inputNode = m_dataFlowRepo->getOrCreateNode(inputVar)) {
                            if (auto readNode = m_dataFlowRepo->createNode(DataFlowNode::Read, output)) {
                                m_dataFlowRepo->addSuccessor(inputNode, readNode);
                                ctx.markValueAsAffected(inputVar);
                            }
                        }
                    }
                }
                else if (unaryOp->getId() == ircode::OperationId::COPY) {
                    if (auto inputVar = std::dynamic_pointer_cast<ircode::Variable>(input)) {
                        if (auto inputNode = m_dataFlowRepo->getOrCreateNode(inputVar)) {
                            auto outputAddrVal = output->getMemAddress().value;
                            if (auto outputAddrVar = std::dynamic_pointer_cast<ircode::Variable>(outputAddrVal)) {
                                if (auto addrNode = m_dataFlowRepo->getOrCreateNode(outputAddrVar)) {
                                    if (auto writeNode = m_dataFlowRepo->createNode(DataFlowNode::Write, output)) {
                                        m_dataFlowRepo->addSuccessor(addrNode, writeNode);
                                        m_dataFlowRepo->addSuccessor(inputNode, writeNode);
                                        ctx.markValueAsAffected(outputAddrVar);
                                        ctx.markValueAsAffected(inputVar);
                                    }
                                }
                            } else {
                                if (auto copyNode = m_dataFlowRepo->createNode(DataFlowNode::Copy, output)) {
                                    m_dataFlowRepo->addSuccessor(inputNode, copyNode);
                                    ctx.markValueAsAffected(inputVar);
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
                    if (auto inputVar1 = std::dynamic_pointer_cast<ircode::Variable>(binaryOp->getInput1())) {
                        if (auto inputNode1 = m_dataFlowRepo->getOrCreateNode(inputVar1)) {
                            if (auto inputVar2 = std::dynamic_pointer_cast<ircode::Variable>(binaryOp->getInput2())) {
                                if (auto inputNode2 = m_dataFlowRepo->getOrCreateNode(inputVar2)) {
                                    if (auto copyNode = m_dataFlowRepo->createNode(DataFlowNode::Copy, output)) {
                                        m_dataFlowRepo->addSuccessor(inputNode1, copyNode);
                                        m_dataFlowRepo->addSuccessor(inputNode2, copyNode);
                                        ctx.markValueAsAffected(inputVar1);
                                        ctx.markValueAsAffected(inputVar2);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    };
};