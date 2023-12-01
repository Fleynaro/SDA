#include "DataFlowResearcherFixture.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

void DataFlowResearcherFixture::SetUp() {
    ResearcherFixture::SetUp();
    dataFlowRepo = std::make_unique<researcher::DataFlowRepository>(context->getEventPipe());
    dataFlowCollector = std::make_unique<researcher::DataFlowCollector>(
        program,
        context->getPlatform(),
        dataFlowRepo.get());
    eventPipe->connect(dataFlowCollector->getEventPipe());
}

::testing::AssertionResult DataFlowResearcherFixture::cmpDataFlow(ircode::Function* function, const std::string& expectedCode) const {
    return CmpDataFlow(dataFlowRepo.get(), function, expectedCode);
}

::testing::AssertionResult DataFlowResearcherFixture::CmpDataFlow(researcher::DataFlowRepository* dataFlowRepo, ircode::Function* function, const std::string& expectedCode) {
    return Compare(researcher::PrintDataFlowForFunction(dataFlowRepo, function), expectedCode);
}