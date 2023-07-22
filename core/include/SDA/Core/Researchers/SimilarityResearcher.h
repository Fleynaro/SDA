#pragma once
#include "ClassResearcher.h"

namespace sda::researcher
{
    class SimilarityResearcher
    {
        ClassRepository* m_classRepo;
        StructureRepository* m_structureRepo;
        DataFlowRepository* m_dataFlowRepo;
        std::set<std::shared_ptr<ircode::Variable>> m_variables;
    public:
        SimilarityResearcher(
            ClassRepository* classRepo,
            StructureRepository* structureRepo,
            DataFlowRepository* dataFlowRepo
        )
            : m_classRepo(classRepo)
            , m_structureRepo(structureRepo)
            , m_dataFlowRepo(dataFlowRepo)
        {}

        const std::set<std::shared_ptr<ircode::Variable>>& getVariables() const {
            return m_variables;
        }

        void research(std::shared_ptr<ircode::Variable> startVariable) {
            std::set<Structure*> visitedStructures;
            std::list<std::shared_ptr<ircode::Variable>> variablesToVisit;
            variablesToVisit.push_back(startVariable);
            m_variables.clear();
            while(!variablesToVisit.empty()) {
                auto it = variablesToVisit.begin();
                auto variable = *it;
                variablesToVisit.erase(it);
                if (m_variables.find(variable) != m_variables.end())
                    continue;
                m_variables.insert(variable);

                // operations
                for (auto op : variable->getOperations()) {
                    if (op->getId() == ircode::OperationId::INT_ADD) {
                        if (auto binaryOp = dynamic_cast<ircode::BinaryOperation*>(op)) {
                            auto input1 = binaryOp->getInput1();
                            auto input2 = binaryOp->getInput1();
                            if (auto inputVar1 = std::dynamic_pointer_cast<ircode::Variable>(input1)) {
                                variablesToVisit.push_back(inputVar1);
                            }
                            if (auto inputVar2 = std::dynamic_pointer_cast<ircode::Variable>(input2)) {
                                variablesToVisit.push_back(inputVar2);
                            }
                            variablesToVisit.push_back(op->getOutput());
                        }
                    }
                }

                // structures
                if (auto node = m_dataFlowRepo->getNode(variable)) {
                    if (auto link = m_structureRepo->getLink(node)) {
                        if (visitedStructures.find(link->structure) == visitedStructures.end()) {
                            std::set<Structure*> structuresInGroup;
                            m_classRepo->gatherStructuresInGroup(link->structure, structuresInGroup);
                            visitedStructures.insert(structuresInGroup.begin(), structuresInGroup.end());
                            for (auto structure : structuresInGroup) {
                                for (auto linkedNode : structure->linkedNodes) {
                                    if (auto linkedVar = linkedNode->getVariable()) {
                                        variablesToVisit.push_back(linkedVar);
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
