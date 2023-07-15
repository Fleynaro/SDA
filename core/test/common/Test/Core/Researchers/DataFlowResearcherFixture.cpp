#include "DataFlowResearcherFixture.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

void DataFlowSemanticsFixture::SetUp() {
    SemanticsFixture::SetUp();
    dataFlowRepo = std::make_unique<semantics::DataFlowRepository>(context->getEventPipe());
    dataFlowCollector = std::make_unique<semantics::DataFlowCollector>(
        program,
        context->getPlatform(),
        dataFlowRepo.get());
    eventPipe->connect(dataFlowCollector->getEventPipe());
}

::testing::AssertionResult DataFlowSemanticsFixture::cmpDataFlow(ircode::Function* function, const std::string& expectedCode) const {
    return CmpDataFlow(dataFlowRepo.get(), function, expectedCode);
}

std::string DataFlowSemanticsFixture::GetNodeName(semantics::DataFlowNode* node, ircode::Function* function) {
    if (auto var = node->getVariable()) {
        auto varFunction = var->getSourceOperation()->getBlock()->getFunction();
        return node->getName(varFunction != function);
    }
    return node->getName();
}

::testing::AssertionResult DataFlowSemanticsFixture::CmpDataFlow(semantics::DataFlowRepository* dataFlowRepo, ircode::Function* function, const std::string& expectedCode) {
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
                ss << GetNodeName(node, function) << " <- Unknown";
                printer.newLine();
            } else {
                for (auto predNode : node->predecessors) {
                    if (auto predVar = predNode->getVariable()) {
                        if (var->getSourceOperation()->getBlock()->getFunction() != function && 
                            predVar->getSourceOperation()->getBlock()->getFunction() != function) continue;
                    }
                    ss << GetNodeName(node, function) << " <- ";
                    if (node->type == semantics::DataFlowNode::Copy)
                        ss << "Copy ";
                    else if (node->type == semantics::DataFlowNode::Write)
                        ss << "Write ";
                    else if (node->type == semantics::DataFlowNode::Read)
                        ss << "Read ";
                    ss << GetNodeName(predNode, function);
                    if (node->offset > 0) {
                        ss << " + 0x" << utils::to_hex() << node->offset;
                    }
                    printer.newLine();
                }
            }
        }
    }
    return Compare(ss.str(), expectedCode);
}