#include "SDA/Core/Researchers/StructureResearcher.h"

using namespace sda;
using namespace sda::researcher;

void Structure::passDescendants(std::function<void(Structure* structure, const std::function<void(Structure* structure)>& next)> callback)
{
    std::map<Structure*, size_t> structKnocks;
    std::list<Structure*> structToVisit;
    structToVisit.push_back(this);
    do {
        while (!structToVisit.empty()) {
            auto structure = structToVisit.front();
            structToVisit.pop_front();
            auto it = structKnocks.find(structure);
            if (it == structKnocks.end()) {
                it = structKnocks.insert({ structure, 0 }).first;
            }
            auto knocks = ++it->second;
            if (knocks < structure->inputs.size()) {
                continue;
            }
            structKnocks.erase(it);
            auto next = [&](Structure* structure) {
                structToVisit.push_front(structure);
            };
            callback(structure, next);

            // debug
            {
                std::stringstream ss;
                std::string sep;
                for (auto structure : structToVisit) {
                    ss << sep << structure->name;
                    sep = ",";
                }
                PLOG_DEBUG << "structure=" << structure->name << ", structToVisit=" << ss.str();
            }
        }
        if (!structKnocks.empty()) {
            auto structure = structKnocks.begin()->first;
            structToVisit.push_back(structure);
        }
    } while (!structToVisit.empty());
}

StructureRepository::StructureRepository(std::shared_ptr<sda::EventPipe> eventPipe)
    : m_eventPipe(eventPipe)
{
    IF_PLOG(plog::debug) {
        m_eventPipe->subscribe(std::function([&](const StructureCreatedEvent& event) {
            PLOG_DEBUG << "StructureCreatedEvent: " << event.structure->name;
        }));
        m_eventPipe->subscribe(std::function([&](const StructureRemovedEvent& event) {
            PLOG_DEBUG << "StructureRemovedEvent: " << event.structure->name;
        }));
        m_eventPipe->subscribe(std::function([&](const ChildAddedEvent& event) {
            PLOG_DEBUG << "ChildAddedEvent: " << event.structure->name << " -> child " << event.child->name;
        }));
        m_eventPipe->subscribe(std::function([&](const ChildRemovedEvent& event) {
            PLOG_DEBUG << "ChildRemovedEvent: " << event.structure->name << " -> child " << event.child->name;
        }));
        m_eventPipe->subscribe(std::function([&](const LinkCreatedEvent& event) {
            std::string offsetStr;
            if (event.offset) {
                offsetStr = " + 0x" + utils::ToHex(event.offset);
            }
            PLOG_DEBUG << "LinkCreatedEvent: node " << event.node->getName()
                        << " -> struct " << event.structure->name << offsetStr;
        }));
    }
}

void StructureResearcher::research(DataFlowNode* node) {
    DataFlowNode::PassSuccessors(node, std::function([this](DataFlowNode* node, bool& goNextNodes, const std::function<void(DataFlowNode* node)>& next) {
        research(node, goNextNodes, next);
    }), m_program->getEventPipe());
}

void StructureResearcher::research(DataFlowNode* node, bool& goNextNodes, const std::function<void(DataFlowNode* node)>& next) {
    auto hash = m_structureRepo->getHash(node);
    researchConstants(node);
    researchStructures(node, next);
    // go next if there are changes (for copy nodes go next always, see test StructureResearcherTest.Functions)
    goNextNodes = hash != m_structureRepo->getHash(node);
    PLOG_DEBUG << "goNextNodes=" << goNextNodes << ", hash=" << hash << ", new hash=" << m_structureRepo->getHash(node);
}

void StructureResearcher::researchConstants(DataFlowNode* node) {
    if (node->type == DataFlowNode::Copy) {
        if (node->predecessors.size() == 1) {
            auto pred = node->predecessors.front();
            if (auto predConstant = m_structureRepo->getConstant(pred)) {
                m_structureRepo->setConstant(node, *predConstant);
            }
        }
    }
    else if (node->type == DataFlowNode::Unknown) {
        if (auto constant = node->getConstant()) {
            // see test StructureResearcherTest.If
            m_structureRepo->setConstant(node, constant->getConstVarnode()->getValue());
        }
    }
}

void StructureResearcher::researchStructures(DataFlowNode* node, const std::function<void(DataFlowNode* node)>& next) {
    if (node->type == DataFlowNode::Copy) {
        bool hasLink = false;
        for (auto pred : node->predecessors) {
            if (m_structureRepo->getLink(pred)) {
                hasLink = true;
                break;
            }
        }
        if (!hasLink) return;
        for (auto pred : node->predecessors) {
            if (!m_structureRepo->getLink(pred)) {
                m_structureRepo->getOrCreateStructure(pred);
            }
        }
        if (node->predecessors.size() == 1) {
            auto pred = node->predecessors.front();
            auto predLink = m_structureRepo->getLink(pred);
            
            bool isNewStructure = false;
            if (node->offset == 0) {
                // gather conditions (see test StructureResearcherTest.If)
                auto variable = node->getVariable();
                assert(variable);
                auto block = variable->getSourceOperation()->getBlock();
                auto structToConditions = findConditions(block);
                auto it = structToConditions.find(predLink->structure);
                if (it != structToConditions.end()) {
                    auto& prevConditions = predLink->structure->conditions;
                    auto& newConditions = it->second;
                    auto newCondHash = newConditions.hash();
                    if (newCondHash != prevConditions.hash()) {
                        auto structure = m_structureRepo->getOrCreateStructure(node);

                        // if predecessor is changed then clean the structure
                        size_t newHash = newCondHash;
                        boost::hash_combine(newHash, predLink->structure->id);
                        auto& prevHash = *m_structureRepo->getHashPtr(node);
                        if (prevHash != newHash) {
                            m_structureRepo->markAsUpdated(structure);
                            m_structureRepo->clearStructure(structure);
                            prevHash = newCondHash;
                        }

                        structure->conditions = newConditions;
                        m_structureRepo->addChild(predLink->structure, structure);
                        m_structureRepo->addOutput(predLink->structure, structure);
                        isNewStructure = true;
                    }
                }
            }

            if (!isNewStructure) {
                if (auto link = m_structureRepo->getLink(node)) {
                    if (link->own) {
                        m_structureRepo->removeStructure(link->structure);
                    }
                }
                m_structureRepo->addLink(node, predLink->structure, predLink->offset + node->offset);
            }
        } else {
            // see test StructureResearcherTest::Functions
            if (auto link = m_structureRepo->getLink(node)) {
                if (!link->own) {
                    // see test StructureResearcherTest.NewFunctionAdded
                    next(link->structure->sourceNode);
                    m_structureRepo->removeStructure(link->structure);
                    return;
                }
            }

            auto structure = m_structureRepo->getOrCreateStructure(node);

            // if one of predecessors is changed then clean the structure
            size_t newHash = 0;
            for (auto pred : node->predecessors) {
                auto predLink = m_structureRepo->getLink(pred);
                boost::hash_combine(newHash, predLink->structure->id);
            }
            auto& prevHash = *m_structureRepo->getHashPtr(node);
            if (prevHash != newHash) {
                m_structureRepo->markAsUpdated(structure);
                m_structureRepo->clearStructure(structure);
                prevHash = newHash;
            }
            // this loop should be executed anyway (not only when predecessors are changed), see test StructureResearcherTest::Functions
            for (auto pred : node->predecessors) {
                auto predLink = m_structureRepo->getLink(pred);
                m_structureRepo->addChild(structure, predLink->structure);
                m_structureRepo->addOutput(predLink->structure, structure);
            }
        }
    }
    else if (node->type == DataFlowNode::Read) {
        assert(node->predecessors.size() == 1);
        auto addrNode = node->predecessors.front();
        auto addrLink = m_structureRepo->getLink(addrNode);
        if (!addrLink) return;
        auto structure = m_structureRepo->getOrCreateStructure(node);
        m_structureRepo->addField(addrLink->structure, addrLink->offset, structure, false);
    }
    else if (node->type == DataFlowNode::Write) {
        assert(node->predecessors.size() == 2);
        auto addrNode = node->predecessors.front();
        auto valueNode = node->predecessors.back();
        auto addrLink = m_structureRepo->getLink(addrNode);
        if (!addrLink) return;
        addrLink->structure->constants.remove(addrLink->offset);
        if (auto valueLink = m_structureRepo->getLink(valueNode)) {
            if (valueLink->offset == 0) {
                m_structureRepo->addField(addrLink->structure, addrLink->offset, valueLink->structure, true);
                m_structureRepo->addLink(node, valueLink->structure);
            } else {
                // see test StructureResearcherTest.GlobalVarPointerAssignment (when offset != 0)
            }
        }
        else if (auto constValue = m_structureRepo->getConstant(valueNode)) {
            // see test StructureResearcherTest.If
            addrLink->structure->constants.insert(addrLink->offset, *constValue);
        }
    }
    else if (node->type == DataFlowNode::Unknown) {
        if (node->getVariable()) {
            m_structureRepo->getOrCreateStructure(node);
        }
    }      
}

std::map<Structure*, ConstantSet> StructureResearcher::findConditions(ircode::Block* block) {
    std::map<Structure*, ConstantSet> result;
    auto conditions = m_constCondRepo->findConditions(block);
    for (auto& [type, variable, value] : conditions) {
        if (type != ConstantCondition::EQUAL)
            continue;
        if (auto loadOp = goToLoadOperation(variable->getSourceOperation())) {
            auto linearExpr = ircode::GetLinearExpr(loadOp->getInput());
            auto offset = linearExpr.getConstTermValue();
            auto baseTerms = ircode::ToBaseTerms(linearExpr, m_platform);
            for (auto& term : baseTerms) {
                if (auto ptrVar = std::dynamic_pointer_cast<ircode::Variable>(term)) {
                    if (auto ptrVarNode = m_dataFlowRepo->getNode(ptrVar)) {
                        if (auto link = m_structureRepo->getLink(ptrVarNode)) {
                            result[link->structure].insert(offset, value);
                        }
                    }
                }
            }
        }
    }
    return result;
}

const ircode::UnaryOperation* StructureResearcher::goToLoadOperation(const ircode::Operation* operation) {
    if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(operation)) {
        if (unaryOp->getId() == ircode::OperationId::COPY || unaryOp->getId() == ircode::OperationId::REF) {
            if (auto var = std::dynamic_pointer_cast<ircode::Variable>(unaryOp->getInput())) {
                return goToLoadOperation(var->getSourceOperation());
            }
        }
        else if (unaryOp->getId() == ircode::OperationId::LOAD) {
            return unaryOp;
        }
    }
    return nullptr;
}
